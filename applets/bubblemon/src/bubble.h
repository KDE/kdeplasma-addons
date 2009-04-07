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

namespace Plasma {
    class Svg;
}

class Bubble : public Plasma::Applet {
    Q_OBJECT
    
    public:
        Bubble(QObject *parent, const QVariantList &args);
        ~Bubble();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void init();
        void createConfigurationInterface(KConfigDialog *parent);
        QString icon() const;

    public slots:
        void dataUpdated(QString name, Plasma::DataEngine::Data data);
        
    protected:
        QPainterPath shape() const;
        void constraintsEvent(Plasma::Constraints constraints);
        void resizeEvent(QGraphicsSceneResizeEvent *evt);
        
    protected slots:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        
    private slots:
        void configAccepted();
        void moveBubbles();
        void showLabel(bool);
        void updateLabelAnimation(qreal trans);
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
        bool m_showingText;
        bool m_animated;
        int m_val;
        int m_max;
        int m_speed;
        int m_animID;
        qreal m_labelTransparency;
        float m_bubbleSpeed;
        float m_bubbleHeight;
        QString m_sensor;
        Plasma::Svg *m_svg;
        QList<QPoint> m_bubbles;
        QString m_label;
        QTimer *m_animator;
        QTimeLine *m_interpolator;
        Plasma::DataEngine *m_engine;
        QStandardItemModel *m_sensorModel;
};

#endif
