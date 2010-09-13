/***************************************************************************
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#include "blackboardwidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <KIO/DeleteJob>
#include <KSaveFile>
#include <KStandardDirs>

#include <Plasma/Applet>
#include <Plasma/Theme>

BlackBoardWidget::BlackBoardWidget(Plasma::Applet *parent)
      : QGraphicsWidget(parent)
{
    m_changed = false;
    setAcceptTouchEvents(true);
    m_parentApplet = parent;
    
    m_color = QColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_oldPoint = QPointF(-1, 0);
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(saveImage()));
    timer->start(600000);

    QTimer::singleShot(500, this, SLOT(loadImage()));
}

BlackBoardWidget::~BlackBoardWidget()
{
}

void BlackBoardWidget::saveImage()
{ 
    if (m_parentApplet->destroyed()){
        KIO::del(imagePath());
    }else{
        if (m_changed){
            KSaveFile imageFile(imagePath());
            imageFile.open();
            m_pixmap.save(&imageFile, "PNG");
            imageFile.finalize();
            imageFile.close();
        }
    }
}

void BlackBoardWidget::loadImage()
{
    m_painter.end();
    m_pixmap.load(imagePath(), "PNG");
    update(contentsRect());
    m_painter.begin(&m_pixmap);
    m_painter.setPen(QPen(m_color, 3));
}

void BlackBoardWidget::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    update(contentsRect());
}

void BlackBoardWidget::drawSegment(QPointF point0, QPointF point1, qreal penRadius)
{
    m_painter.setPen(QPen(m_color, penRadius));
    m_painter.drawLine(point0, point1);

    qreal x = qMin(point0.x(), point1.x()) -(penRadius + 1);
    qreal y = qMin(point0.y(), point1.y()) -(penRadius + 1);
    qreal w = qMax(point0.x(), point1.x()) + penRadius + 1 - x;
    qreal h = qMax(point0.y(), point1.y()) + penRadius + 1 - y;

    update(x,y,w,h);
    m_changed = true;  
}

bool BlackBoardWidget::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        QList<QTouchEvent::TouchPoint> touchPoints = static_cast<QTouchEvent *>(event)->touchPoints();
        foreach (const QTouchEvent::TouchPoint &touchPoint, touchPoints) {
            switch (touchPoint.state()) {
            case Qt::TouchPointStationary:
                // don't do anything if this touch point hasn't moved
                continue;
            default:
                drawSegment(touchPoint.lastPos(), touchPoint.pos(), 3*touchPoint.pressure());
                break;
            }
        }
        break;
    }
    default:
        return QGraphicsWidget::event(event);
    }
    return true;
}

void BlackBoardWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QPointF lastPos = event->lastPos();

    if (m_oldPoint.x() != -1){
        drawSegment(m_oldPoint, lastPos, 1);      
    }

    m_oldPoint = lastPos;
}

void BlackBoardWidget::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    mouseMoveEvent(event);
    m_oldPoint.setX(-1);
}

void BlackBoardWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    
    if (m_painter.isActive())
        m_painter.end();
    
    QPixmap tmpPixmap = m_pixmap;
    m_pixmap = QPixmap(contentsRect().width(), contentsRect().height());
    m_pixmap.fill(Qt::transparent);

    m_painter.begin(&m_pixmap);
    m_painter.drawPixmap(0, 0, tmpPixmap);
    m_painter.setPen(QPen(m_color, 3));
}

void BlackBoardWidget::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)  
  
    p->drawPixmap(option->exposedRect, m_pixmap, option->exposedRect);
}

void BlackBoardWidget::setBrushColor(QColor color)
{
    m_color = color;
    m_painter.setPen(QPen(m_color, 3));
}

void BlackBoardWidget::erase()
{
    m_pixmap.fill(Qt::transparent);
    update(contentsRect());
    KIO::del(imagePath());
}

QString BlackBoardWidget::imagePath()
{
    return KStandardDirs::locateLocal("data", QLatin1String("plasma-desktop-datastorage/blackboard-") + QString::number(m_parentApplet->id()) + QLatin1String(".png")); 
}

#include "blackboardwidget.moc"
