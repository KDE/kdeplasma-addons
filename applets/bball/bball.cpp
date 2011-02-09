/***************************************************************************
 *   Copyright 2008 by Thomas Gillespie <tomjamesgillespie@googlemail.com> *
 *   Copyright 2010 by Enrico Ros <enrico.ros@gmail.com>                   *
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
#include <QtCore/QSizeF>
#include <QtGui/QPainter>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <KSharedConfig>
#include <KLocale>
#include <KStandardDirs>
#include <KIO/NetAccess>
#include <KMessageBox>

#include <stdlib.h>

#ifdef Q_CC_MSVC
#include <iso646.h>
#endif

// default values
static const int initial_ball_radius = 64;

using namespace Plasma;

bballApplet::bballApplet(QObject * parent, const QVariantList & args):
    Plasma::Applet(parent, args),
    // keep this in sync with readConfiguration's default values
    m_overlay_enabled(false),
    m_overlay_opacity(0),
    m_gravity(1.5),
    m_friction(0.03),
    m_restitution(0.8),
    m_sound_enabled(false),
    m_sound_volume(100),
    m_auto_bounce_enabled(false),
    m_auto_bounce_strength(0),
    // more status
    m_radius(initial_ball_radius),
    m_angle(0),
    m_angularVelocity(0),
    m_mousePressed(false),
    m_soundPlayer(0),
    m_audioOutput(0)
{
    setHasConfigurationInterface(true);
    //TODO figure out why it is not good enough to set it here
    // but that it needs to be set in constraintsEvent
    // see also the icon applet
    // update: apparently it gets reset when the formfactor changes
    // this can be caught in constraintsEvent with Plasma::FormFactorConstraint
    setBackgroundHints(NoBackground);
    resize(contentSizeHint());
}

void bballApplet::init()
{
    configChanged();
    // monitor the scene for size changes (so we change the bouncing rect)
    if (scene())
        connect(scene(), SIGNAL(sceneRectChanged(QRectF)), this, SLOT(updateScreenRect()));
    m_timer.start(25, this);
}

void bballApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem * option, const QRect & contentsRect)
{
    Q_UNUSED(option);
    Q_UNUSED(contentsRect);

    if (m_ballPixmap.isNull())
        return;

    if (m_angle) {
        p->translate(m_radius, m_radius);
        p->rotate(360 * m_angle / 6.28);
        p->translate(-m_radius, -m_radius);
        if (m_velocity.length() < 300) {
            p->setRenderHint(QPainter::SmoothPixmapTransform);
            p->setRenderHint(QPainter::Antialiasing);
        }
    }
    p->drawPixmap(QPoint(0, 0), m_ballPixmap);
}

QSizeF bballApplet::contentSizeHint() const
{
    return QSizeF(m_radius * 2, m_radius * 2);
}

void bballApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);

    // Appearance
    ui.imageUrl->setUrl(KUrl::fromPath(m_image_url));
    ui.colourizeEnabled->setChecked(m_overlay_enabled);
    ui.colourizeLabel->setEnabled(m_overlay_enabled);
    ui.colourize->setEnabled(m_overlay_enabled);
    ui.colourize->setColor(m_overlay_colour);
    ui.colourizeOpacityLabel->setEnabled(m_overlay_enabled);
    ui.colourizeOpacitySlider->setEnabled(m_overlay_enabled);
    ui.colourizeOpacitySlider->setSliderPosition(static_cast< int >(m_overlay_opacity/2.55)-1);

    // Physics
    ui.gravity->setSliderPosition(static_cast< int >(m_gravity * 100));
    ui.friction->setSliderPosition(static_cast< int >(m_friction * 100));
    ui.resitution->setSliderPosition(static_cast < int >(m_restitution * 100));

    // Sound
    ui.soundEnabled->setChecked(m_sound_enabled);
    ui.soundVolumeLabel->setEnabled(m_sound_enabled);
    ui.soundVolume->setEnabled(m_sound_enabled);
    ui.soundVolume->setSliderPosition(m_sound_volume);
    ui.soundFileLabel->setEnabled(m_sound_enabled);
    ui.soundFile->setEnabled(m_sound_enabled);
    ui.soundFile->setUrl(KUrl::fromPath(m_sound_url));

    // Misc
    ui.autoBounceEnabled->setChecked(m_auto_bounce_enabled);
    ui.autoBounceStrengthLabel->setEnabled(m_auto_bounce_enabled);
    ui.autoBounceStrength->setValue(static_cast < int >(m_auto_bounce_strength));
    ui.autoBounceStrength->setEnabled(m_auto_bounce_enabled);

    parent->addPage(widget, i18n("General"), icon());
    connect(ui.imageUrl, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.colourizeEnabled, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.colourize, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
    connect(ui.colourizeOpacitySlider, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.gravity, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.friction, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.resitution, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.soundEnabled, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.soundVolume, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.soundFile, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.autoBounceEnabled, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.autoBounceStrength, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(parent, SIGNAL(accepted()), this, SLOT(configurationChanged()));
}

void bballApplet::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (immutability() != Plasma::Mutable)
        return;

    if (m_geometry.isNull())
        syncGeometry();

    // reset timing
    m_timer.stop();
    m_time = QTime();
    update();

    // reset physics
    m_velocity = QVector2D();
    m_angularVelocity = 0;

    // save mouse position
    m_mouseScenePos = event->scenePos();
    m_mousePressed = true;

    event->accept();
}

void bballApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (immutability() != Plasma::Mutable)
        return;

    // TODO: use real timing instead of the fixed step (1/0.025)
    m_velocity = QVector2D(m_mouseScenePos - m_prevMouseScenePos) / 0.025;
    m_mousePressed = false;
    m_timer.start(25, this);
    event->accept();
}

void bballApplet::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (immutability() != Plasma::Mutable)
        return;

    m_prevMouseScenePos = m_mouseScenePos;
    m_mouseScenePos = event->scenePos();
    m_geometry.translate(m_mouseScenePos - m_prevMouseScenePos);
    setGeometry(m_geometry);
    event->accept();
}

void bballApplet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        updatePhysics();
        return;
    }
    Applet::timerEvent(event);
}

void bballApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::LocationConstraint)
        m_geometry = QRectF();

    if (constraints & Plasma::FormFactorConstraint)
        setBackgroundHints(NoBackground);

    if (constraints & Plasma::SizeConstraint)
        syncGeometry();
}

void bballApplet::updateScreenRect()
{
    m_screenRect = QDesktopWidget().availableGeometry();
    m_timer.start(25, this);
}

void bballApplet::configurationChanged()
{
    KConfigGroup cg = config();

    // Appearance
    if (KIO::NetAccess::exists(ui.imageUrl->url(), KIO::NetAccess::SourceSide, NULL)) {
        m_image_url = ui.imageUrl->url().path();
        cg.writeEntry("ImgURL", m_image_url);
        m_ballSvg.setImagePath(m_image_url);
    } else
        KMessageBox::error(0, i18n("The given image could not be loaded. The image will not be changed."));
    m_overlay_enabled = ui.colourizeEnabled->checkState() == Qt::Checked;
    cg.writeEntry("OverlayEnabled", m_overlay_enabled);
    m_overlay_colour = ui.colourize->color();
    cg.writeEntry("OverlayColour", m_overlay_colour);
    m_overlay_opacity = static_cast< int >(ui.colourizeOpacitySlider->value() * 2.55);
    cg.writeEntry("OverlayOpacity", m_overlay_opacity);
    updateScaledBallImage();

    // Physics
    m_gravity = (qreal)ui.gravity->value() / 100.0;
    cg.writeEntry("Gravity", m_gravity);
    m_friction = (qreal)ui.friction->value() / 100.0;
    cg.writeEntry("Friction", 1.0 - m_friction); // RETROCOMP
    m_restitution = ui.resitution->value () / 100.0;
    cg.writeEntry("Resitution", m_restitution);

    // Sound
    m_sound_enabled = ui.soundEnabled->checkState() == Qt::Checked;
    cg.writeEntry("SoundEnabled", m_sound_enabled);
    if (m_sound_enabled) {
        if (KIO::NetAccess::exists(ui.soundFile->url(), KIO::NetAccess::SourceSide, NULL)) {
            m_sound_url = ui.soundFile->url().path();
            cg.writeEntry("SoundURL", m_sound_url);
            if (m_soundPlayer)
                m_soundPlayer->setCurrentSource(m_sound_url);
        } else
            KMessageBox::error(0, i18n("The given sound could not be loaded. The sound will not be changed."));
    }
    m_sound_volume = ui.soundVolume->value();
    cg.writeEntry("SoundVolume", m_sound_volume);
    if (m_audioOutput)
        m_audioOutput->setVolume(m_sound_volume);

    // Misc
    m_auto_bounce_enabled = ui.autoBounceEnabled->checkState() == Qt::Checked;
    cg.writeEntry("AutoBounceEnabled", m_auto_bounce_enabled);
    m_auto_bounce_strength = ui.autoBounceStrength->value();
    cg.writeEntry ("AutoBounceStrength", m_auto_bounce_strength);
    if (m_auto_bounce_enabled || m_gravity > 0)
        m_timer.start(25, this);

    // mouse - undo the mouse clicked
    m_mousePressed = false;

    update();
}

void bballApplet::configChanged()
{
    KConfigGroup cg = config ();

    // Appearance
    m_image_url = cg.readEntry("ImgURL", KStandardDirs::locate("data", QLatin1String( "bball/bball.svgz" )));
    m_overlay_enabled = cg.readEntry("OverlayEnabled", false);
    m_overlay_colour = cg.readEntry("OverlayColour", QColor(Qt::white));
    m_overlay_opacity = cg.readEntry("OverlayOpacity", 0);
    m_ballSvg.setImagePath(m_image_url);
    updateScaledBallImage();

    // Physics
    m_gravity = cg.readEntry("Gravity", 1.5);
    m_friction = 1.0 - cg.readEntry("Friction", 0.97); // RETROCOMP
    m_restitution = cg.readEntry("Resitution", 0.8);

    // Sound
    m_sound_enabled = cg.readEntry("SoundEnabled", false);
    m_sound_url = cg.readEntry("SoundURL", KStandardDirs::locate ("data", QLatin1String( "bball/bounce.ogg" )));
    m_sound_volume = cg.readEntry("SoundVolume", 100);

    // Misc
    m_auto_bounce_enabled = cg.readEntry("AutoBounceEnabled", false);
    m_auto_bounce_strength = cg.readEntry("AutoBounceStrength", 0);
}

void bballApplet::updatePhysics()
{
    // find out the delta-time since the last call
    if (m_time.isNull())
        m_time.start();
    qreal dT = qMin((qreal)m_time.restart() / 1000.0, 0.5);

    // skip progessing if externally moved (disabled because plasma moves this too frequently)
    //if (m_geometry != geometry())
    //    m_geometry = QRectF();

    // skip if dragging
    if (m_mousePressed || m_geometry.isNull() || m_radius < 1)
        return;

    if (m_screenRect.isNull())
        updateScreenRect();

    // add some randomness if autobouncing
    if (m_auto_bounce_enabled && rand() < RAND_MAX/35) {
        m_velocity += QVector2D(
                (rand() - RAND_MAX/2) * m_auto_bounce_strength * 0.0000005,
                (rand() - RAND_MAX/2) * m_auto_bounce_strength * 0.0000005);
    }

    // update velocity and position
    m_velocity += QVector2D(0, (qreal)m_screenRect.height() * m_gravity * dT);
    m_velocity *= (1.0 - 2 * m_friction * dT);
    m_geometry.translate((m_velocity * dT).toPointF());

    // floor
    bool collision = false;
    bool bottom = false;
    if (m_geometry.bottom() >= m_screenRect.bottom() && m_velocity.y() > 0) {
        m_geometry.moveBottom(m_screenRect.bottom());
        m_velocity *= QVector2D(1, -m_restitution);
        m_angularVelocity = m_velocity.x() / m_radius;
        collision = true;
        bottom = true;
    }

    // ceiling
    if (m_geometry.top() <= m_screenRect.top() && m_velocity.y() < 0) {
        m_geometry.moveTop(m_screenRect.top ());
        m_velocity *= QVector2D(1, -m_restitution);
        m_angularVelocity = -m_velocity.x() / m_radius;
        collision = true;
    }

    // right
    if (m_geometry.right() >= m_screenRect.right() && m_velocity.x() > 0) {
        m_geometry.moveRight(m_screenRect.right() - 0.1);
        m_velocity *= QVector2D(-m_restitution, 1);
        m_angularVelocity = -m_velocity.y() / m_radius;
        if (bottom)
            m_velocity.setX(0);
        collision = true;
    }

    // left
    if (m_geometry.left() <= m_screenRect.left() && m_velocity.x() < 0) {
        m_geometry.moveLeft(m_screenRect.left () + 0.1);
        m_velocity *= QVector2D(-m_restitution, 1);
        m_angularVelocity = m_velocity.y() / m_radius;
        if (bottom)
            m_velocity.setX(0);
        collision = true;
    }

    m_angularVelocity *= (0.9999 - 2 * m_friction * dT);
    m_angle += m_angularVelocity * dT;

    // stop animation if reached bottom and still
    if (m_velocity.length() < 10.0 && qAbs(m_angularVelocity) < 0.1 && !m_auto_bounce_enabled) {
        m_timer.stop();
        update();
        return;
    }

    // move this and update graphics
    setGeometry(m_geometry);
    update();

    if (collision)
        playBoingSound();
}

void bballApplet::playBoingSound()
{
    if (!m_sound_enabled || m_velocity.x() == 0.0 || m_velocity.y() == 0.0)
        return;

    // create the player if missing
    if (!m_soundPlayer) {
        m_soundPlayer = new Phonon::MediaObject(this);
        m_soundPlayer->setCurrentSource(m_sound_url);
        m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        m_audioOutput->setVolume(m_sound_volume);
        createPath(m_soundPlayer, m_audioOutput);
    }

    // play the sound
    m_soundPlayer->seek(0);
    m_soundPlayer->play();
}

void bballApplet::syncGeometry()
{
    m_geometry = geometry();
    m_radius = static_cast<int>(geometry().width()) / 2;
    updateScaledBallImage();
}

void bballApplet::updateScaledBallImage()
{
    // regen m_ballPixmap
    m_ballSvg.resize(contentSizeHint());
    m_ballPixmap = m_ballSvg.pixmap();

    // tint the pixmap if requested
    if (m_overlay_enabled) {
        QPainter p(&m_ballPixmap);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        QColor brush = m_overlay_colour;
        brush.setAlpha(m_overlay_opacity);
        p.setBrush(brush);
        p.drawEllipse(QRectF(0, 0, m_radius * 2, m_radius * 2));
    }
}

#include "bball.moc"
