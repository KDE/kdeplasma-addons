/***************************************************************************
 *   Copyright 2008 by Thomas Gillespie <tomjamesgillespie@googlemail.com> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "bball.h"
#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <QMessageBox>
#include <qdesktopwidget.h>
#include <KSharedConfig>
#include <KGlobalSettings>
#include <KLocale>
#include <KStandardDirs>
#include <KIO/NetAccess>
#include <KMessageBox>
#include <QDebug>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/animator.h>

#include <stdlib.h>

#ifdef Q_CC_MSVC
#include <iso646.h>
#endif

const int ball_size = 100;
const int ball_radius = ball_size / 2;
const int ball_circum = (int) (2.0 * 3.14158 * ball_radius);

using namespace Plasma;

bballApplet::bballApplet (QObject * parent, const QVariantList & args):
  Plasma::Applet(parent, args),
  m_radius(50),
  m_angle(0.0),
  m_ball_img(),
  m_x_vel(0.0),
  m_y_vel(0.0),
  m_circum_vel(0.0),
  m_mouse_pressed(false),
  m_bottom_left(0),
  m_bottom_right(0),
  m_bottom(0)
{
  setHasConfigurationInterface (true);
  //TODO figure out why it is not good enough to set it here
  // but that it needs to be set in constraintsEvent
  // see also the icon applet
  // update: apparently it gets reset when the formfactor changes
  // this can be caught in constraintsEvent with Plasma::FormFactorConstraint
  setBackgroundHints(NoBackground);
  resize(125, 125);
}

bballApplet::~bballApplet ()
{
}

void bballApplet::init ()
{
  readConfiguration();

  m_sound = new Phonon::MediaObject (this);
  audioOutput = new Phonon::AudioOutput (Phonon::MusicCategory, this);
  if (m_sound_enabled)
  {
    m_sound->setCurrentSource (m_sound_url);
    audioOutput->setVolume (m_sound_volume);
  }
  createPath (m_sound, audioOutput);

  m_position = QRectF (geometry ().x (), geometry ().y (), 100, 100);
  QDesktopWidget *desktop = new QDesktopWidget ();
  m_screen = desktop->availableGeometry ();

  //timer
  m_timer = new QTimer (this);
  connect (m_timer, SIGNAL (timeout ()), this, SLOT (goPhysics ()));
  m_timer->start (25);
  delete desktop;
}

void bballApplet::readConfiguration()
{
  KConfigGroup cg = config ();

  //Appearance
  m_image_url = cg.readEntry ("ImgURL", KStandardDirs::locate ("data", "bball/bball.svgz"));
  m_ball_img.setImagePath(m_image_url);
  m_overlay_enabled = cg.readEntry ("OverlayEnabled", false);
  m_overlay_colour = cg.readEntry ("OverlayColour", QColor ());
  m_overlay_opacity = cg.readEntry ("OverlayOpactiy", 0);
  kDebug() << m_overlay_enabled;
  kDebug() << m_overlay_colour;
  kDebug() << m_overlay_opacity;
  updateScaledBallImage();

  //Physics
  m_gravity = cg.readEntry ("Gravity", 1.5);
  m_friction = cg.readEntry ("Friction", 0.97);
  m_resitution = cg.readEntry ("Resitution", 0.8);

  //Sound
  m_sound_enabled = cg.readEntry ("SoundEnabled", false);
  m_sound_url = cg.readEntry ("SoundURL", KStandardDirs::locate ("data", "bball/bounce.ogg"));
  m_sound_volume = cg.readEntry ("SoundVolume", 100);

  //Misc
  m_auto_bounce_enabled = cg.readEntry ("AutoBounceEnabled", false);
  m_auto_bounce_strength = cg.readEntry ("AutoBounceStrength", 0);
}

void bballApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    ui.setupUi (widget);

    connect(parent, SIGNAL(accepted()), this, SLOT(configAccepted()));

    //Appearance
    ui.imageUrl->setUrl (KUrl::fromPath (m_image_url));
    ui.colourizeEnabled->setChecked (m_overlay_enabled);
    ui.colourizeLabel->setEnabled (m_overlay_enabled);
    ui.colourize->setEnabled (m_overlay_enabled);
    ui.colourize->setColor (m_overlay_colour);
    ui.colourizeOpacityLabel->setEnabled (m_overlay_enabled);
    ui.colourizeOpacitySlider->setEnabled (m_overlay_enabled);
    ui.colourizeOpacitySlider->setSliderPosition (static_cast < int >(m_overlay_opacity/2.55)-1);

    //Physics
    ui.gravity->setSliderPosition (static_cast < int >(m_gravity * 100));
    ui.friction->setSliderPosition (static_cast < int >(100 - m_friction * 100));
    ui.resitution->setSliderPosition (static_cast < int >(m_resitution * 100));

    //Sound
    ui.soundEnabled->setChecked (m_sound_enabled);
    ui.soundVolumeLabel->setEnabled (m_sound_enabled);
    ui.soundVolume->setEnabled (m_sound_enabled);
    ui.soundVolume->setSliderPosition (m_sound_volume);
    ui.soundFileLabel->setEnabled (m_sound_enabled);
    ui.soundFile->setEnabled (m_sound_enabled);
    ui.soundFile->setUrl (KUrl::fromPath (m_sound_url));

    //Misc
    ui.autoBounceEnabled->setChecked (m_auto_bounce_enabled);
    ui.autoBounceStrengthLabel->setEnabled (m_auto_bounce_enabled);
    ui.autoBounceStrength->setValue (static_cast < int >(m_auto_bounce_strength));
    ui.autoBounceStrength->setEnabled (m_auto_bounce_enabled);
    parent->addPage(widget, i18n("General"), icon());
}

void bballApplet::configAccepted ()
{
  KConfigGroup cg = config ();

  //Appearance
  if(KIO::NetAccess::exists(ui.imageUrl->url (), KIO::NetAccess::SourceSide, NULL))
  {
    m_image_url = ui.imageUrl->url ().path ();
    m_ball_img.setImagePath(m_image_url);
    cg.writeEntry ("ImgURL", m_image_url);
  }
  else
  {
    KMessageBox::error(0, i18n("The given image could not be loaded. The image will not be changed."));
  }
  m_overlay_enabled = ui.colourizeEnabled->checkState () == Qt::Checked;
  cg.writeEntry ("OverlayEnabled", m_overlay_enabled);
  m_overlay_colour = ui.colourize->color ();
  cg.writeEntry ("OverlayColour", m_overlay_colour);
  m_overlay_opacity = static_cast < int > (ui.colourizeOpacitySlider->value() * 2.55);
  kDebug() << "Colour: h-" << m_overlay_colour.hueF() << " s-" << m_overlay_colour.saturationF() << " v-" << m_overlay_colour.valueF() << endl;
  cg.writeEntry ("OverlayOpacity", m_overlay_opacity);
  m_overlay_colour.setAlpha (m_overlay_opacity);
  updateScaledBallImage();

  //Physics
  m_gravity = ui.gravity->value () / 100.0;
  cg.writeEntry ("Gravity", m_gravity);
  m_friction = (100 - ui.friction->value ()) / 100.0;
  cg.writeEntry ("Friction", m_friction);
  m_resitution = ui.resitution->value () / 100.0;
  cg.writeEntry ("Resitution", m_resitution);

  //Sound
  m_sound_enabled = (ui.soundEnabled->checkState () == Qt::Checked);
  cg.writeEntry ("SoundEnabled", m_sound_enabled);
  if ( m_sound_enabled )
  {
    if(KIO::NetAccess::exists(ui.soundFile->url (), KIO::NetAccess::SourceSide, NULL))
    {
      m_sound_url = ui.soundFile->url ().path ();
      cg.writeEntry ("SoundURL", m_sound_url);
      m_sound->setCurrentSource (m_sound_url);
    }
    else
    {
      KMessageBox::error(0, i18n("The given sound could not be loaded. The sound will not be changed."));
    }
    m_sound_volume = ui.soundVolume->value ();
    cg.writeEntry ("SoundVolume", m_sound_volume);
    audioOutput->setVolume (m_sound_volume);
  }

  //Misc
  m_auto_bounce_enabled = ui.autoBounceEnabled->checkState () == Qt::Checked;
  cg.writeEntry ("AutoBounceEnabled", m_auto_bounce_enabled);
  m_auto_bounce_strength = ui.autoBounceStrength->value ();
  cg.writeEntry ("AutoBounceStrength", m_auto_bounce_strength);
  if ( m_auto_bounce_enabled)
  {
    m_timer->start ();
  }

  //mouse - undo the mouse clicked
  m_mouse_pressed = false;

  update();
}

QSizeF bballApplet::contentSizeHint() const
{
    return QSizeF(m_radius*2, m_radius*2);
}

void bballApplet::constraintsEvent (Plasma::Constraints constraints)
{
  setBackgroundHints(NoBackground);
  if ( constraints & Plasma::SizeConstraint )
  {
    m_position =  QRectF (geometry ().x (), geometry ().y (), geometry ().width (), geometry ().height ());
    m_radius = static_cast<int>(geometry().width()) / 2;
    updateScaledBallImage();
  }
}

void bballApplet::updateScaledBallImage()
{

  kDebug() << "Updating Ball";
  kDebug() << m_overlay_enabled;
  kDebug() << m_overlay_colour;
  kDebug() << m_overlay_opacity;

  m_ball_img.resize( m_radius * 2, m_radius * 2 );
  m_pixmap = m_ball_img.pixmap();
  if (m_overlay_enabled)
  {
    QPainter p(&m_pixmap);
    p.setPen (QColor (0, 0, 0, 0));
    p.setBrush (m_overlay_colour);
    p.drawPie (m_pie_size, 0, 5760);
  }
  m_pie_size = QRectF( 0, 0, m_radius * 2, m_radius * 2 );
}

inline void bballApplet::adjustAngularVelocity( double * velocity, double circ_velocity )
{
  // make sure angular velocity doesn't infinitely accelerate
  if (qAbs (*velocity + circ_velocity) <= qAbs (circ_velocity))
  {
    *velocity = circ_velocity;
  }
}

inline void bballApplet::bottomCollision()
{
  m_position.moveBottom (m_screen.bottom ());
  m_y_vel *= -m_resitution;
  adjustAngularVelocity( &m_x_vel, m_circum_vel );
  m_circum_vel = m_x_vel;
  m_bottom=1;
}

inline void bballApplet::topCollision()
{
  m_position.moveTop (m_screen.top ());
  m_y_vel *= -m_resitution;
  adjustAngularVelocity( &m_x_vel, -m_circum_vel );
  m_circum_vel = -m_x_vel;
}

inline void bballApplet::rightCollision()
{
  m_position.moveRight (m_screen.right () - 0.1);
  m_x_vel *= -m_resitution;
  adjustAngularVelocity( &m_y_vel, -m_circum_vel );
  m_circum_vel = -m_y_vel;
  if (m_bottom == 1) 
  {
    //kDebug() << "HIT Bottom AND RIGHT";
    m_x_vel = 0.0;
    m_bottom=0;
    m_bottom_right=1;
  }
}

inline void bballApplet::leftCollision()
{
  m_position.moveLeft (m_screen.left () + 0.1);
  m_x_vel *= -m_resitution;
  adjustAngularVelocity( &m_y_vel, m_circum_vel );
  m_circum_vel = m_y_vel;
  if (m_bottom == 1) 
  {
    //kDebug() << "HIT Bottom AND LEFT";
    m_bottom=0;
    m_bottom_left=1;
  }
}

inline void bballApplet::checkCollisions()
{
  bool collision = false;

  //floor
  if (m_position.bottom () >= m_screen.bottom ())
  {
    bottomCollision();
    collision = true;
    kDebug() << "Bottom collision";
  }

  //ceiling
  if (m_position.top () <= m_screen.top ())
  {
    topCollision();
    collision = true;
  }

  //right
  if (m_position.right () >= m_screen.right ())
  {
    rightCollision();
    collision = true;
  }

  //left
  if (m_position.left () <= m_screen.left ())
  {
    leftCollision();
    collision = true;
  }

  if ( collision )
  {
    boing();
  }
}

inline void bballApplet::applyPhysics()
{
  m_y_vel += m_gravity;
  m_y_vel *= m_friction;
  m_x_vel *= m_friction;
  if (m_bottom_right == 1 || m_bottom_left == 1) {
    m_x_vel=0.0;
  }
}

inline void bballApplet::moveAndRotateBall()
{
  m_position.translate (m_x_vel, m_y_vel);
  m_angle += (m_circum_vel / ball_circum) * 360.0;
  setGeometry(m_position);
}

void bballApplet::goPhysics ()
{
  if ( m_mouse_pressed ) {
    return;
  }

  if ( m_x_vel < 1.0 and m_y_vel < 1.0 and m_position.bottom() >= m_screen.bottom() and !m_auto_bounce_enabled )
  {
    m_timer->stop();
    return;
  }
  //add some randomness, maybe
  if(rand() < RAND_MAX/35 and m_auto_bounce_enabled)
  {
    int randx, randy; //lol
    randx = rand();
    randy = rand();
    m_x_vel += (randx - RAND_MAX/2) * m_auto_bounce_strength * 0.000000005;
    m_y_vel += (randy - RAND_MAX/2) * m_auto_bounce_strength * 0.000000005;
  }

  checkCollisions();

  applyPhysics();
  moveAndRotateBall();
}


void bballApplet::boing ()
{
  if (m_sound_enabled and m_x_vel != 0 and m_y_vel != 0)
  {
    m_sound->seek(0);
    m_sound->play();
  }
}

void bballApplet::paintInterface (QPainter * p, const QStyleOptionGraphicsItem * option, const QRect & contentsRect)
{
  Q_UNUSED (option);
  Q_UNUSED( contentsRect );

// INFO - Not needed, if ball is still plasma does not paint, and if this is in it messes up repaintting if, for example, the mouse is hovered over it.
//  if (m_x_vel == 0 and m_y_vel == 0 and not m_mouse_pressed)
//  {
//   return;
//  }

  p->setRenderHint (QPainter::SmoothPixmapTransform);
  p->setRenderHint (QPainter::Antialiasing);

  p->translate(m_radius, m_radius);
  p->rotate(m_angle);
  p->translate(-m_radius, -m_radius);
  p->drawPixmap(QPoint(0, 0), m_pixmap);
}

void bballApplet::mousePressEvent (QGraphicsSceneMouseEvent * event)
{
  m_x_vel = 0;
  m_y_vel = 0;
  m_circum_vel = 0;
  m_mouse = event->scenePos();
  m_mouse_pressed = true;
  event->accept();
}

void bballApplet::mouseReleaseEvent (QGraphicsSceneMouseEvent * event)
{
  m_x_vel = (m_mouse - m_old_mouse).x();
  m_y_vel = (m_mouse - m_old_mouse).y();
  m_mouse_pressed = false;
  m_timer->start();
  event->accept();
}

void bballApplet::mouseMoveEvent (QGraphicsSceneMouseEvent * event)
{
  m_old_mouse = m_mouse;
  m_mouse = event->scenePos();
  m_position.translate(m_mouse - m_old_mouse);
  setGeometry(m_position);
  event->accept();
}

#include "bball.moc"
