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

#ifndef bball_HEADER
#define bball_HEADER

#include <QtCore/QBasicTimer>
#include <QtCore/QTime>
#include <QtGui/QVector2D>
#include <KConfigDialog>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

#include "ui_bballConfig.h"

class QGraphicsSceneMouseEvent;
class QSizeF;

class bballApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    bballApplet(QObject * parent, const QVariantList & args);

    // ::Plasma::Applet
    void init();
    void paintInterface(QPainter * painter, const QStyleOptionGraphicsItem * option, const QRect & contentsRect);
    QSizeF contentSizeHint() const;
    void createConfigurationInterface(KConfigDialog *parent);

    // ::QGraphicsItem
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void timerEvent(QTimerEvent *event);

protected:
    // ::Plasma::Applet
    void constraintsEvent(Plasma::Constraints constraints);

protected Q_SLOTS:
    void updateScreenRect();
    void configurationChanged();

public slots:
    void configChanged();
    
private:
    void updatePhysics();
    void playBoingSound();
    void syncGeometry();
    void updateScaledBallImage();

    // config values
    QString m_image_url;
    bool m_overlay_enabled;
    int m_overlay_opacity;
    QColor m_overlay_colour;

    qreal m_gravity, m_friction, m_restitution;

    bool m_sound_enabled;
    int m_sound_volume;
    QString m_sound_url;

    bool m_auto_bounce_enabled;
    qreal m_auto_bounce_strength;

    // status
    QBasicTimer m_timer;
    QTime m_time;

    QRectF m_screenRect;

    int m_radius;
    QRectF m_geometry;
    QVector2D m_velocity;

    qreal m_angle;
    qreal m_angularVelocity;

    Plasma::Svg m_ballSvg;
    QPixmap m_ballPixmap;

    bool m_mousePressed;
    QPointF m_mouseScenePos;
    QPointF m_prevMouseScenePos;

    Ui::bballConfig ui;

    Phonon::MediaObject * m_soundPlayer;
    Phonon::AudioOutput * m_audioOutput;
};

K_EXPORT_PLASMA_APPLET (BbalL, bballApplet)
#endif
