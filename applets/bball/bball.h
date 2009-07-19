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

#ifndef bball_HEADER
#define bball_HEADER

#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <KIcon>
#include <KConfigDialog>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

#include "ui_bballConfig.h"

class QSizeF;

class bballApplet : public Plasma::Applet
{
    Q_OBJECT
public:
  bballApplet (QObject * parent, const QVariantList & args);
  ~bballApplet ();
  void init ();
  void paintInterface (QPainter * painter, const QStyleOptionGraphicsItem * option, const QRect & contentsRect);
  QSizeF contentSizeHint () const;

  void mousePressEvent (QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent (QGraphicsSceneMouseEvent * event);
  void mouseMoveEvent (QGraphicsSceneMouseEvent * event);

  public slots:
    void goPhysics ();
    void createConfigurationInterface(KConfigDialog *parent);

  protected slots:
    void configAccepted ();

  protected:
    void constraintsEvent (Plasma::Constraints constraints);

private:
  void boing ();
  void updateScaledBallImage();
  void readConfiguration();
  inline void adjustAngularVelocity( double * velocity, double circ_velocity );
  inline void bottomCollision();
  inline void topCollision();
  inline void leftCollision();
  inline void rightCollision();
  inline void checkCollisions();
  inline void applyPhysics();
  inline void moveAndRotateBall();

  int m_radius, m_old_radius, m_sound_volume, m_overlay_opacity, m_bottom_left, m_bottom_right, m_bottom;
  QRectF m_position, m_screen, m_pie_size;
  QPointF m_old_mouse, m_mouse;
  qreal m_angle;
  Plasma::Svg m_ball_img;
  QPixmap m_pixmap;
  double m_x_vel, m_y_vel, m_gravity, m_resitution, m_friction, m_circum_vel, m_auto_bounce_strength;
  bool m_mouse_pressed, m_sound_enabled, m_auto_bounce_enabled, m_overlay_enabled;
  QTimer *m_timer;
  //Config dialog
  Ui::bballConfig ui;
  QString m_image_url, m_sound_url;
  QColor m_overlay_colour;
  Phonon::AudioOutput * audioOutput;
  Phonon::MediaObject * m_sound;
};

K_EXPORT_PLASMA_APPLET (BbalL, bballApplet)
#endif
