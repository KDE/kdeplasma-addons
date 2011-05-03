/*
*   Copyright 2009 by Trever Fischer <wm161@wm161.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef BUBBLE_H
#define BUBBLE_H

#include <Plasma/Applet>
#include <Plasma/DataEngine>

#include "ui_settings.h"

class QTimeLine;
class QGraphicsSceneResizeEvent;
class QTimer;
class QStandardItemModel;
class QPropertyAnimation;

namespace Plasma {
    class Svg;
}

class Bubble : public Plasma::Applet {
    Q_OBJECT
    Q_PROPERTY(qreal labelTransparency READ labelTransparency WRITE setLabelTransparency)
    
    public:
        Bubble(QObject *parent, const QVariantList &args);
        ~Bubble();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void init();
        void createConfigurationInterface(KConfigDialog *parent);
        QString icon() const;
        void setLabelTransparency(qreal);
        qreal labelTransparency() const;

    public slots:
        void dataUpdated(QString name, Plasma::DataEngine::Data data);
        void configChanged();

    protected:
        QPainterPath shape() const;
        void constraintsEvent(Plasma::Constraints constraints);
        void resizeEvent(QGraphicsSceneResizeEvent *evt);
        
    protected slots:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        
    private slots:
        void writeConfig();
        void moveBubbles();
        void showLabel(bool);
        void interpolateValue();
        void connectSensor();
        void reconnectSensor();
        void disconnectSensor();
        void reloadTheme();
        void repaintNeeded();
        
    private:
        void drawLabel(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF &contentsRect);
        
        Ui::Settings ui;
        bool m_showText;
        bool m_animated;
        int m_val;
        int m_max;
        int m_speed;
        QVector<QPoint> m_bubbles;
        int m_bubbleCount;
        qreal m_labelTransparency;
        float m_bubbleSpeed;
        QSizeF m_bubbleRect;
        QString m_sensor;
        Plasma::Svg *m_svg;
        QString m_label;
        QTimer *m_animator;
        QTimeLine *m_interpolator;
        Plasma::DataEngine *m_engine;
        QStandardItemModel *m_sensorModel;
        QPropertyAnimation *m_animation;
        QRectF m_clip;
        QPainterPath m_bubbleClip;
        bool m_rebuildClip;
};

#endif
