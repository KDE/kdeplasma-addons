/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
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

#include "handle.h"

#include <QApplication>
#include <QBitmap>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QTouchEvent>
#include <QMatrix>
#include <QTransform>
#include <QWeakPointer>
#include <QPropertyAnimation>

#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kwindowsystem.h>

#include <cmath>
#include <math.h>

#include <Plasma/Corona>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/View>
#include <Plasma/FrameSvg>

#include "abstractgroup.h"
#include "groupingcontainment.h"

using namespace Plasma;
qreal _k_distanceForPoint(QPointF point);
qreal _k_pointAngle(QPointF point);
QPointF _k_rotatePoint(QPointF point, qreal angle);
QPointF _k_projectPoint(QPointF point, QPointF v);

Handle::Handle(GroupingContainment *parent, Plasma::Applet *applet, const QPointF &hoverPos)
      : QGraphicsObject(applet),
        m_pressedButton(NoButton),
        m_containment(parent),
        m_applet(applet),
        m_group(0),
        m_widget(applet),
        m_iconSize(KIconLoader::SizeSmall),
        m_opacity(0.0),
        m_animType(FadeIn),
        m_backgroundBuffer(0),
        m_currentView(applet->view()),
        m_entryPos(hoverPos),
        m_buttonsOnRight(false),
        m_pendingFade(false)
{
    init();
}

Handle::Handle(GroupingContainment *parent, AbstractGroup *group, const QPointF &hoverPos)
      : QGraphicsObject(group),
        m_pressedButton(NoButton),
        m_containment(parent),
        m_applet(0),
        m_group(group),
        m_widget(group),
        m_iconSize(KIconLoader::SizeSmall),
        m_opacity(0.0),
        m_animType(FadeIn),
        m_backgroundBuffer(0),
        m_currentView(group->view()),
        m_entryPos(hoverPos),
        m_buttonsOnRight(false),
        m_pendingFade(false)
{
    init();
}

Handle::~Handle()
{
    detachWidget();
    delete m_backgroundBuffer;
}

void Handle::init()
{
    setFlags(flags() | QGraphicsItem::ItemStacksBehindParent);
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View,
                             Theme::defaultTheme()->colorScheme());
    setAcceptTouchEvents(true);
    m_gradientColor = colorScheme.background(KColorScheme::NormalBackground).color();
    m_originalGeom = mapToScene(QRectF(QPoint(0,0), m_widget->size())).boundingRect();
    m_originalTransform = m_widget->transform();

    QPointF center = QRectF(QPointF(), m_widget->size()).center();
    m_angle = _k_pointAngle(m_originalTransform.map(center + QPointF(1.0, 0.0)) - center);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(333);

    m_leaveTimer = new QTimer(this);
    m_leaveTimer->setSingleShot(true);
    m_leaveTimer->setInterval(500);

    connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    connect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    connect(m_widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed()));

    setAcceptsHoverEvents(true);
    m_hoverTimer->start();

    //icons
    m_configureIcons = new Svg(this);
    m_configureIcons->setImagePath("widgets/configuration-icons");
    m_configureIcons->setContainsMultipleImages(true);

    m_background = new FrameSvg(this);
    m_background->setImagePath("widgets/background");
    m_widget->installSceneEventFilter(this);
}

QGraphicsWidget *Handle::widget() const
{
    return m_widget;
}

void Handle::detachWidget()
{
    if (!m_widget) {
        return;
    }

    disconnect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    disconnect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    m_widget->disconnect(this);

    if (m_applet && (m_applet->geometry() != m_originalGeom || m_applet->transform() != m_originalTransform)) {
//         emit m_applet->appletTransformedByUser(); //FIXME: protected!
    } else if (m_group && (m_group->geometry() != m_originalGeom || m_group->transform() != m_originalTransform)) {
        emit m_group->groupTransformedByUser();
    }

    m_applet = 0;
    m_group = 0;
    m_widget = 0;
}

QRectF Handle::boundingRect() const
{
    return m_totalRect;
}

QPainterPath Handle::shape() const
{
    //when the containment changes the widget is reset to 0
    if (m_widget) {
        QPainterPath path = PaintUtils::roundedRectangle(m_decorationRect, 10);
        return path.united(m_widget->shape());
    } else {
        return QGraphicsItem::shape();
    }
}

QPainterPath handleRect(const QRectF &rect, int radius, bool onRight)
{
    QPainterPath path;
    if (onRight) {
        // make the left side straight
        path.moveTo(rect.left(), rect.top());              // Top left
        path.lineTo(rect.right() - radius, rect.top());    // Top side
        path.quadTo(rect.right(), rect.top(),
                    rect.right(), rect.top() + radius);    // Top right corner
        path.lineTo(rect.right(), rect.bottom() - radius); // Right side
        path.quadTo(rect.right(), rect.bottom(),
                    rect.right() - radius, rect.bottom()); // Bottom right corner
        path.lineTo(rect.left(), rect.bottom());           // Bottom side
    } else {
        // make the right side straight
        path.moveTo(QPointF(rect.left(), rect.top() + radius));
        path.quadTo(rect.left(), rect.top(),
                    rect.left() + radius, rect.top());     // Top left corner
        path.lineTo(rect.right(), rect.top());             // Top side
        path.lineTo(rect.right(), rect.bottom());          // Right side
        path.lineTo(rect.left() + radius, rect.bottom());  // Bottom side
        path.quadTo(rect.left(), rect.bottom(),
                    rect.left(), rect.bottom() - radius);  // Bottom left corner
    }

    path.closeSubpath();
    return path;
}

void Handle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //kDebug() << m_opacity << m_anim << FadeOut;
    if (qFuzzyCompare(m_opacity + 1.0, 1.0)) {
        if (m_animType == FadeOut) {
            //kDebug() << "WOOOOOOOOO";
            QTimer::singleShot(0, this, SLOT(emitDisappear()));
        }
        return;
    }

    qreal translation;

    if (m_buttonsOnRight) {
        //kDebug() << "translating by" << m_opacity
        //         << (-(1 - m_opacity) * m_rect.width()) << m_rect.width();
        translation = -(1 - m_opacity) * m_rect.width();
    } else {
        translation = (1 - m_opacity) * m_rect.width();
    }

    painter->translate(translation, 0);

    painter->setPen(Qt::NoPen);
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);

    int iconMargin = m_iconSize / 2;

    const QSize pixmapSize(int(m_decorationRect.width()),
                           int(m_decorationRect.height()) + m_iconSize * 5 + 1);
    const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    bool isRunning = false;
    if (m_anim.data()) {
        isRunning = m_anim.data()->state() == QAbstractAnimation::Running ? \
                    true : false;
    }

    //regenerate our buffer?
    if (isRunning || !m_backgroundBuffer || m_backgroundBuffer->size() != pixmapSize) {
        QColor transparencyColor = Qt::black;
        transparencyColor.setAlphaF(qMin(m_opacity, qreal(0.99)));

        QLinearGradient g(QPoint(0, 0), QPoint(m_decorationRect.width(), 0));
        //fading out panel
        if (m_rect.height() > qreal(minimumHeight()) * 1.25) {
            if (m_buttonsOnRight) {
                qreal opaquePoint =
                    (m_background->marginSize(LeftMargin) - translation) / m_decorationRect.width();
                //kDebug() << "opaquePoint" << opaquePoint
                //         << m_background->marginSize(LeftMargin) << m_decorationRect.width();
                g.setColorAt(0.0, Qt::transparent);
                g.setColorAt(qMax(0.0, opaquePoint - 0.05), Qt::transparent);
                g.setColorAt(opaquePoint, transparencyColor);
                g.setColorAt(1.0, transparencyColor);
            } else {
                qreal opaquePoint =
                    1 - ((m_background->marginSize(RightMargin) + translation) / m_decorationRect.width());
                g.setColorAt(1.0, Qt::transparent);
                g.setColorAt(opaquePoint + 0.05, Qt::transparent);
                g.setColorAt(qMax(qreal(0), opaquePoint), transparencyColor);
                g.setColorAt(0.0, transparencyColor);
            }
        //complete panel
        } else {
            g.setColorAt(0.0, transparencyColor);
        }

        m_background->resizeFrame(m_decorationRect.size());

        if (!m_backgroundBuffer || m_backgroundBuffer->size() != pixmapSize) {
            delete m_backgroundBuffer;
            m_backgroundBuffer = new QPixmap(pixmapSize);
        }
        m_backgroundBuffer->fill(Qt::transparent);
        QPainter buffPainter(m_backgroundBuffer);

        m_background->paintFrame(&buffPainter);

        //+1 because otherwise due to rounding errors when rotated could appear one pixel
        //of the icon at the border of the widget
        //QRectF iconRect(QPointF(pixmapSize.width() - m_iconSize + 1, m_iconSize), iconSize);
        QRectF iconRect(QPointF(0, m_decorationRect.height() + 1), iconSize);
        if (m_buttonsOnRight) {
            iconRect.moveLeft(
            pixmapSize.width() - m_iconSize - m_background->marginSize(LeftMargin));
        } else {
            iconRect.moveLeft(m_background->marginSize(RightMargin));
        }
        AbstractGroup *group = m_widget->property("group").value<AbstractGroup *>();
        if (!(group && group->groupType() == AbstractGroup::ConstrainedGroup)) {
            if (m_buttonsOnRight) {
                m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tr2bl");
            } else {
                m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tl2br");
            }
            iconRect.translate(0, m_iconSize);

            m_configureIcons->paint(&buffPainter, iconRect, "rotate");
            iconRect.translate(0, m_iconSize);
        }

        if (m_applet && m_applet->hasConfigurationInterface()) {
            m_configureIcons->paint(&buffPainter, iconRect, "configure");
            iconRect.translate(0, m_iconSize);
        }

        if (m_applet && m_applet->hasValidAssociatedApplication()) {
            m_configureIcons->paint(&buffPainter, iconRect, "maximize");
            iconRect.translate(0, m_iconSize);
        }

        m_configureIcons->paint(&buffPainter, iconRect, "close");

        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        //blend the background
        buffPainter.fillRect(m_backgroundBuffer->rect(), g);
        //blend the icons
        //buffPainter.fillRect(QRect(QPoint((int)m_decorationRect.width(), 0), QSize(m_iconSize + 1,
        //                                  (int)m_decorationRect.height())), transparencyColor);
    }

    painter->drawPixmap(m_decorationRect.toRect(), *m_backgroundBuffer,
                        QRect(QPoint(0, 0), m_decorationRect.size().toSize()));

    //XXX this code is duplicated in the next function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_MARGIN, iconMargin);
    QPointF step = QPointF(0, m_iconSize + iconMargin);
    QPointF separator = step + QPointF(0, iconMargin);
    //end duplicate code

    QPointF shiftC;
    QPointF shiftD;
    QPointF shiftR;
    QPointF shiftM;
    QPointF shiftMx;

    switch(m_pressedButton)
    {
    case ConfigureButton:
        shiftC = QPointF(2, 2);
        break;
    case RemoveButton:
        shiftD = QPointF(2, 2);
        break;
    case RotateButton:
        shiftR = QPointF(2, 2);
        break;
    case ResizeButton:
        shiftM = QPointF(2, 2);
        break;
    case MaximizeButton:
        shiftMx = QPointF(2, 2);
        break;
    default:
        break;
    }

    QRectF sourceIconRect(QPointF(0, m_decorationRect.height() + 1), iconSize);
    if (m_buttonsOnRight) {
        sourceIconRect.moveLeft(
            pixmapSize.width() - m_iconSize - m_background->marginSize(LeftMargin));
    } else {
        sourceIconRect.moveLeft(m_background->marginSize(RightMargin));
    }

    AbstractGroup *group = m_widget->property("group").value<AbstractGroup *>();
    if (!(group && group->groupType() == AbstractGroup::ConstrainedGroup)) {
        if (m_group || (m_applet && m_applet->aspectRatioMode() != FixedSize)) {
            //resize
            painter->drawPixmap(
                QRectF(basePoint + shiftM, iconSize), *m_backgroundBuffer, sourceIconRect);
            basePoint += step;
        }
        sourceIconRect.translate(0, m_iconSize);

        //rotate
        painter->drawPixmap(QRectF(basePoint + shiftR, iconSize), *m_backgroundBuffer, sourceIconRect);
        sourceIconRect.translate(0, m_iconSize);
        basePoint += step;
    }
    //configure
    if (m_applet && m_applet->hasConfigurationInterface()) {
        painter->drawPixmap(
            QRectF(basePoint + shiftC, iconSize), *m_backgroundBuffer, sourceIconRect);
        sourceIconRect.translate(0, m_iconSize);
        basePoint += step;
    }

    //maximize
    if (m_applet && m_applet->hasValidAssociatedApplication()) {
        painter->drawPixmap(
            QRectF(basePoint + shiftMx, iconSize), *m_backgroundBuffer, sourceIconRect);
        sourceIconRect.translate(0, m_iconSize);
        basePoint += step;
    }

    //close
    basePoint = m_rect.bottomLeft() + QPointF(HANDLE_MARGIN, 0) - step;
    painter->drawPixmap(QRectF(basePoint + shiftD, iconSize), *m_backgroundBuffer, sourceIconRect);
}

void Handle::emitDisappear()
{
    emit disappearDone(this);
}

Handle::ButtonType Handle::mapToButton(const QPointF &point) const
{
    int iconMargin = m_iconSize / 2;
    //XXX this code is duplicated in the prev. function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_MARGIN, iconMargin);
    QPointF step = QPointF(0, m_iconSize + iconMargin);
    QPointF separator = step + QPointF(0, iconMargin);
   //end duplicate code

    QRectF activeArea = QRectF(basePoint, QSizeF(m_iconSize, m_iconSize));

    AbstractGroup *group = m_widget->property("group").value<AbstractGroup *>();
    if (!(group && group->groupType() == AbstractGroup::ConstrainedGroup)) {
        if (m_group || (m_applet && m_applet->aspectRatioMode() != FixedSize)) {
            if (activeArea.contains(point)) {
                return ResizeButton;
            }
            activeArea.translate(step);
        }

        if (activeArea.contains(point)) {
            return RotateButton;
        }
        activeArea.translate(step);
    }

    if (m_applet && m_applet->hasConfigurationInterface()) {
        if (activeArea.contains(point)) {
            return ConfigureButton;
        }
        activeArea.translate(step);
    }

    if (m_applet && m_applet->hasValidAssociatedApplication()) {
        if (activeArea.contains(point)) {
            return MaximizeButton;
        }
    }

    activeArea.moveTop(m_rect.bottom() - activeArea.height() - iconMargin);
    if (activeArea.contains(point)) {
        return RemoveButton;
    }

    return MoveButton;
    //return m_applet->mapToParent(m_applet->shape()).contains(point) ? NoButton : MoveButton;
}

void Handle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //containment recently switched?
    if (!m_widget) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    if (m_pendingFade) {
        //m_pendingFade = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressedButton = mapToButton(event->pos());
        //kDebug() << "button pressed:" << m_pressedButton;
        if (m_pressedButton != NoButton) {
            m_containment->raise(m_widget);
            m_zValue = m_widget->zValue();
            setZValue(m_zValue);
        }

        if (m_pressedButton == MoveButton) {
            m_containment->setMovingWidget(m_widget);
        }

        if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
            m_originalGeom = mapToScene(QRectF(QPoint(0,0), m_widget->size())).boundingRect();
            m_origWidgetCenter = m_originalGeom.center();
            m_origWidgetSize = QPointF(m_widget->size().width(), m_widget->size().height());

            // resize
            if (m_buttonsOnRight) {
                m_resizeStaticPoint = m_widget->mapToScene(QPointF(0, m_widget->size().height()));
            } else {
                m_resizeStaticPoint = m_widget->mapToScene(m_origWidgetSize);
            }
            m_resizeGrabPoint = event->scenePos();

            // rotate
            m_rotateAngleOffset = m_angle - _k_pointAngle(event->scenePos() - m_origWidgetCenter);
        }

        event->accept();

        //set mousePos to the position in the widget, in screencoords, so it becomes easy
        //to reposition the toplevel view to the correct position.
        if (m_currentView && m_widget) {
            QPoint localpos = m_currentView->mapFromScene(m_widget->scenePos());
            m_mousePos = event->screenPos() - m_currentView->mapToGlobal(localpos);
        }
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

bool Handle::leaveCurrentView(const QPoint &pos) const
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (widget->geometry().contains(pos)) {
            //is this widget a plasma view, a different view then our current one,
            //AND not a dashboardview?
            Plasma::View *v = qobject_cast<Plasma::View *>(widget);
            if (v && v != m_currentView && v->containment() != m_containment) {
                return true;
            }
        }
    }
    return false;
}

void Handle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug() << "button pressed:" << m_pressedButton << ", fade pending?" << m_pendingFade;

    if (m_pendingFade) {
        startFading(FadeOut, m_entryPos);
        m_pendingFade = false;
    }

    ButtonType releasedAtButton = mapToButton(event->pos());

    if (m_widget && event->button() == Qt::LeftButton) {
        switch (m_pressedButton) {
        case ConfigureButton:
            //FIXME: Remove this call once the configuration management change was done
            if (m_applet && m_pressedButton == releasedAtButton) {
                m_applet->showConfigurationInterface();
            }
            break;
        case RemoveButton:
            if (m_pressedButton == releasedAtButton) {
                forceDisappear();
                if (m_applet) {
                    m_applet->destroy();
                } else {
                    m_group->destroy();
                }
                return;
            }
            break;
        case MoveButton:
        {
                // test for containment change
                //kDebug() << "testing for containment change, sceneBoundingRect = "
                //         << m_containment->sceneBoundingRect();
                if (!m_containment->sceneBoundingRect().contains(m_widget->scenePos())) {
                    // see which containment it belongs to
                    Corona * corona = qobject_cast<Corona*>(scene());
                    if (corona) {
                        foreach (Containment *containment, corona->containments()) {
                            QPointF pos;
                            QGraphicsView *v = containment->view();
                            if (v) {
                                pos = v->mapToScene(v->mapFromGlobal(event->screenPos() - m_mousePos));

                                if (containment->sceneBoundingRect().contains(pos)) {
                                    //kDebug() << "new containment = " << containments[i];
                                    //kDebug() << "rect = " << containments[i]->sceneBoundingRect();
                                    // add the widget to the new containment and take it from the old one
                                    //kDebug() << "moving to other containment with position" << pos;;
                                    switchContainment(static_cast<GroupingContainment *>(containment), pos);
                                    break;
                                }
                            }
                        }
                    }
                }

                emit widgetMoved(m_widget);

            break;
        }
        case MaximizeButton:
            if (m_applet) {
                m_applet->runAssociatedApplication();
            }
            break;
        default:
            break;
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(QPointF point)
{
    return std::sqrt(point.x() * point.x() + point.y() * point.y());
}

qreal _k_pointAngle(QPointF point)
{
    qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
    qreal cosine = point.x() / r;

    if (point.y() >= 0) {
        return acos(cosine);
    } else {
        return -acos(cosine);
    }
}

QPointF _k_rotatePoint(QPointF point, qreal angle)
{
    return QTransform().rotateRadians(angle).map(point);
}

QPointF _k_projectPoint(QPointF point, QPointF v)
{
    v /= sqrt(v.x() * v.x() + v.y() * v.y());
    qreal a = v.x() * v.x();
    qreal b = v.x() * v.y();
    qreal d = v.y() * v.y();
    return QMatrix(a, b, b, d, 0., 0.).map(point);
}

void Handle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = M_PI_2 /* $i 3.14159 / 2.0 */;

    if (!m_widget) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    //Track how much the mouse has moved.
    QPointF deltaScene  = event->scenePos() - event->lastScenePos();

    if (m_pressedButton == MoveButton) {
        if (leaveCurrentView(event->screenPos())) {
            Plasma::View *v = Plasma::View::topLevelViewAt(event->screenPos());
            if (v && v != m_currentView) {
                Containment *c = v->containment();
                if (c) {
                    QPoint pos = v->mapFromGlobal(event->screenPos());
                    //we actually have been dropped on another containment, so
                    //move there: we have a screenpos, we need a scenepos
                    //FIXME how reliable is this transform?
                    switchContainment(static_cast<GroupingContainment *>(c), v->mapToScene(pos));
                }
            }
        }

        if (m_widget) {
            QPointF mappedPoint = transform().map(QPointF(deltaScene.x(), deltaScene.y()));
            m_widget->moveBy(mappedPoint.x(), mappedPoint.y());
        }
    } else if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
        QPointF cursorPoint = event->scenePos();

        // the code below will adjust these based on the type of operation
        QPointF newSize;
        QPointF newCenter;
        qreal newAngle;

        // get size limits
        QSizeF min = m_widget->minimumSize();
        QSizeF max = m_widget->maximumSize();

        if (min.width() < KIconLoader::SizeSmall || min.height() <  KIconLoader::SizeSmall) {
            min = m_widget->effectiveSizeHint(Qt::MinimumSize);
        }

        if (max.isEmpty()) {
            max = m_widget->effectiveSizeHint(Qt::MaximumSize);
        }

        // If the widget doesn't have a minimum size, calculate based on a
        // minimum content area size of 16x16 (KIconLoader::SizeSmall)
        if (min.width() < KIconLoader::SizeSmall || min.height() <  KIconLoader::SizeSmall) {
            min = m_widget->boundingRect().size() - m_widget->contentsRect().size();
            min = QSizeF(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        }

        if (m_pressedButton == RotateButton) {
            newSize = m_origWidgetSize;
            newCenter = m_origWidgetCenter;

            QPointF centerRelativePoint = cursorPoint - m_origWidgetCenter;
            if (_k_distanceForPoint(centerRelativePoint) < 10) {
                newAngle = m_angle;
            } else {
                qreal cursorAngle = _k_pointAngle(centerRelativePoint);
                newAngle = m_rotateAngleOffset + cursorAngle;
                if (fabs(remainder(newAngle, snapAngle)) < 0.15) {
                    newAngle = newAngle - remainder(newAngle, snapAngle);
                }
            }
        } else {
            // un-rotate screen points so we can read differences of coordinates
            QPointF rStaticPoint = _k_rotatePoint(m_resizeStaticPoint, -m_angle);
            QPointF rCursorPoint = _k_rotatePoint(cursorPoint, -m_angle);
            QPointF rGrabPoint = _k_rotatePoint(m_resizeGrabPoint, -m_angle);

            if (m_buttonsOnRight) {
                newSize = m_origWidgetSize + QPointF(rCursorPoint.x() - rGrabPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            } else {
                newSize = m_origWidgetSize + QPointF(rGrabPoint.x() - rCursorPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            }

            // preserving aspect ratio?
            if (m_applet && ((m_applet->aspectRatioMode() != Plasma::IgnoreAspectRatio &&
                 !(event->modifiers() & Qt::ControlModifier)) ||
                 (m_applet->aspectRatioMode() == Plasma::IgnoreAspectRatio &&
                  (event->modifiers() & Qt::ControlModifier)))) {
                // project size to keep ratio
                newSize = _k_projectPoint(newSize, m_origWidgetSize);
                // limit size, presering ratio
                qreal ratio = m_origWidgetSize.y() / m_origWidgetSize.x();
                newSize.rx() = qMin(max.width(), qMax(min.width(), newSize.x()));
                newSize.ry() = newSize.x() * ratio;
                newSize.ry() = qMin(max.height(), qMax(min.height(), newSize.y()));
                newSize.rx() = newSize.y() / ratio;
            } else {
                // limit size
                newSize.rx() = qMin(max.width(), qMax(min.width(), newSize.x()));
                newSize.ry() = qMin(max.height(), qMax(min.height(), newSize.y()));
            }

            // move center such that the static corner remains in the same place
            if (m_buttonsOnRight) {
                newCenter =  _k_rotatePoint(QPointF(rStaticPoint.x() + newSize.x()/2,
                            rStaticPoint.y() - newSize.y()/2), m_angle);
            } else {
                newCenter =  _k_rotatePoint(QPointF(rStaticPoint.x() - newSize.x()/2,
                            rStaticPoint.y() - newSize.y()/2), m_angle);
            }

            newAngle = m_angle;
        }

        // apply size
        m_widget->resize(newSize.x(), newSize.y());
        // apply position, no need if we're rotating
        if (m_pressedButton != RotateButton) {
            m_widget->setPos(m_widget->parentItem()->mapFromScene(newCenter - newSize/2));
        }

        // apply angle
        QTransform at;
        at.translate(newSize.x()/2, newSize.y()/2);
        at.rotateRadians(newAngle);
        at.translate(-newSize.x()/2, -newSize.y()/2);
        m_widget->setTransform(at);
        m_angle = newAngle;
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

bool Handle::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchEnd: {
        QTransform t = m_widget->transform();
        QRectF geom = m_widget->geometry();
        QPointF translation(t.m31(), t.m32());
        QPointF center = geom.center();
        geom.setWidth(geom.width()*qAbs(t.m11()));
        geom.setHeight(geom.height()*qAbs(t.m22()));
        geom.moveCenter(center);

        m_widget->setGeometry(geom);
        t.reset();
        t.translate(m_widget->size().width()/2, m_widget->size().height()/2);
        t.rotateRadians(m_angle);
        t.translate(-m_widget->size().width()/2, -m_widget->size().height()/2);


        m_widget->setTransform(t);
        return true;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    {
        QList<QTouchEvent::TouchPoint> touchPoints = static_cast<QTouchEvent *>(event)->touchPoints();
        if (touchPoints.count() == 2) {
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

            //rotation
            QLineF line0(touchPoint0.lastScenePos(), touchPoint1.lastScenePos());
            QLineF line1(touchPoint0.scenePos(), touchPoint1.scenePos());
            m_angle = m_angle+(line1.angleTo(line0)*M_PI_2/90);
            QTransform t = m_widget->transform();
            t.translate(m_widget->size().width()/2, m_widget->size().height()/2);
            t.rotate(line1.angleTo(line0));

            //scaling
            qreal scaleFactor = 1;
            if (line0.length() > 0) {
                scaleFactor = line1.length() / line0.length();
            }

            t.scale(scaleFactor, scaleFactor);
            t.translate(-m_widget->size().width()/2, -m_widget->size().height()/2);
            m_widget->setTransform(t);

        }
        return true;
    }
    default:
        break;
    }
    return QGraphicsItem::sceneEvent(event);
}

//pos relative to scene
void Handle::switchContainment(GroupingContainment *containment, const QPointF &pos)
{
    m_containment = containment;
    if (m_applet) {
        Applet *applet = m_applet;
        m_applet = 0; //make sure we don't try to act on the applet again
        applet->removeSceneEventFilter(this);
        forceDisappear(); //takes care of event filter and killing handle
        applet->disconnect(this); //make sure the applet doesn't tell us to do anything
        //applet->setZValue(m_zValue);
        containment->addApplet(applet, containment->mapFromScene(pos), false);
    } else {
        AbstractGroup *group = m_group;
        m_group = 0; //make sure we don't try to act on the applet again
        group->removeSceneEventFilter(this);
        forceDisappear(); //takes care of event filter and killing handle
        group->disconnect(this); //make sure the applet doesn't tell us to do anything
        //applet->setZValue(m_zValue);
        containment->addGroup(group, containment->mapFromScene(pos));
    }

    deleteLater();
}

void Handle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover enter";

    //if a disappear was scheduled stop the timer
    if (m_leaveTimer->isActive()) {
        m_leaveTimer->stop();
    }
    // if we're already fading out, fade back in
    else if (!m_anim.data() && m_animType == FadeOut) {
        startFading(FadeIn, m_entryPos, true);
    }
}

void Handle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEnterEvent(event);
}

void Handle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
      QMenu *menu = qobject_cast<QMenu*>(widget);
      if (menu && menu->isVisible()) {
          connect(menu, SIGNAL(aboutToHide()), this, SLOT(leaveTimeout()));
          return;
      }
    }


    // if we haven't even showed up yet, remove the handle
    if (m_hoverTimer->isActive()) {
        m_hoverTimer->stop();
        QTimer::singleShot(0, this, SLOT(emitDisappear()));
    } else if (m_pressedButton != NoButton) {
        m_pendingFade = true;
    } else {
        //wait a moment to hide the handle in order to recheck the mouse position
        m_leaveTimer->start();
    }
}

bool Handle::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == m_widget && event->type() == QEvent::GraphicsSceneHoverLeave) {
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent*>(event));
    }

    return false;
}

void Handle::setFadeAnimation(qreal progress)
{
    m_opacity = progress;
    //kDebug() << "progress" << progress << "m_opacity" << m_opacity << m_anim << "(" << FadeIn << ")";
    if (qFuzzyCompare(progress, qreal(1.0))) {
        delete m_backgroundBuffer;
        m_backgroundBuffer = 0;
    }

    update();
}

qreal Handle::fadeAnimation() const
{
    return m_opacity;
}

void Handle::hoverTimeout()
{
    startFading(FadeIn, m_entryPos);
}

void Handle::leaveTimeout()
{
    if (!isUnderMouse()) {
        startFading(FadeOut, m_entryPos);
    }
}

void Handle::widgetDestroyed()
{
    m_applet = 0;
    m_group = 0;
    m_widget = 0;
}

void Handle::widgetResized()
{
    prepareGeometryChange();
    calculateSize();
    update();
}

void Handle::setHoverPos(const QPointF &hoverPos)
{
    m_entryPos = hoverPos;
}

void Handle::startFading(FadeType anim, const QPointF &hoverPos, bool preserveSide)
{
    QPropertyAnimation *propAnim = m_anim.data();

    if (anim == FadeIn) {
        if (propAnim) {
            propAnim->stop();
        } else {
            propAnim = new QPropertyAnimation(this, "fadeAnimation", this);
            m_anim = propAnim;
        }
    }

    m_entryPos = hoverPos;
    qreal time = 100;

    if (!m_widget) {
        m_animType = FadeOut;
        setFadeAnimation(1.0);
        return;
    }

    if (anim == FadeIn) {
        //kDebug() << m_entryPos.x() << m_widget->pos().x();
        prepareGeometryChange();
        bool wasOnRight = m_buttonsOnRight;
        if (!preserveSide) {
            m_buttonsOnRight = m_entryPos.x() > (m_widget->size().width() / 2);
        }
        calculateSize();
        QPolygonF region = m_widget->mapToParent(m_rect).intersected(m_widget->parentWidget()->boundingRect());
        //kDebug() << region << m_rect << mapToParent(m_rect) << containmnet->boundingRect();
        if (region != m_widget->mapToParent(m_rect)) {
            // switch sides
            //kDebug() << "switch sides";
            m_buttonsOnRight = !m_buttonsOnRight;
            calculateSize();
            QPolygonF region2 = m_widget->mapToParent(m_rect).intersected(m_widget->parentWidget()->boundingRect());
            if (region2 != mapToParent(m_rect)) {
                // ok, both sides failed to be perfect... which one is more perfect?
                QRectF f1 = region.boundingRect();
                QRectF f2 = region2.boundingRect();
                //kDebug() << "still not a perfect world"
                //         << f2.width() << f2.height() << f1.width() << f1.height();
                if ((f2.width() * f2.height()) < (f1.width() * f1.height())) {
                    //kDebug() << "we did better the first time";
                    m_buttonsOnRight = !m_buttonsOnRight;
                    calculateSize();
                }
            }
        }

        if (wasOnRight != m_buttonsOnRight &&
            m_animType == FadeIn &&
            anim == FadeIn &&
            m_opacity <= 1) {
            m_opacity = 0.0;
        }

        time *= 1.0 - m_opacity;
    } else {
        time *= m_opacity;
    }

    if (propAnim) {
        propAnim->setStartValue(0);
        propAnim->setEndValue(1);
        propAnim->setDuration(time);
    }

    m_animType = anim;
    //kDebug() << "animating for " << time << "ms";
    if (m_animType == FadeIn) {
        propAnim->setDirection(QAbstractAnimation::Forward);
        propAnim->start();
    } else if (propAnim) {
        propAnim->setDirection(QAbstractAnimation::Backward);
        propAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void Handle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut, m_entryPos);
}

int Handle::minimumHeight()
{
    int iconMargin = m_iconSize / 2;
    int requiredHeight =  iconMargin  + //first margin
                          (m_iconSize + iconMargin) * 4 + //XXX remember to update this if the number of buttons changes
                          iconMargin ;  //blank space before the close button

    if (m_applet && m_applet->hasConfigurationInterface()) {
        requiredHeight += (m_iconSize + iconMargin);
    }

    return requiredHeight;
}

void Handle::calculateSize()
{
    KIconLoader *iconLoader = KIconLoader::global();
    //m_iconSize = iconLoader->currentSize(KIconLoader::Small); //does not work with double sized icon
    m_iconSize = iconLoader->loadIcon("transform-scale", KIconLoader::Small).width(); //workaround

    int handleHeight = qMax(minimumHeight(), int(m_widget->contentsRect().height() * 0.8));
    int handleWidth = m_iconSize + 2 * HANDLE_MARGIN;
    int top =
        m_widget->contentsRect().top() + (m_widget->contentsRect().height() - handleHeight) / 2.0;

    qreal marginLeft, marginTop, marginRight, marginBottom;
    m_background->getMargins(marginLeft, marginTop, marginRight, marginBottom);

    if (m_buttonsOnRight) {
        //put the rect on the right of the widget
        m_rect = QRectF(m_widget->size().width(), top, handleWidth, handleHeight);
    } else {
        //put the rect on the left of the widget
        m_rect = QRectF(-handleWidth, top, handleWidth, handleHeight);
    }

    if (m_widget->contentsRect().height() > qreal(minimumHeight()) * 1.25) {
        int addedMargin = marginLeft / 2;

        // now we check to see if the shape is smaller than the contents,
        // and that the shape is not just the bounding rect; in those cases
        // we have a shaped guy and we draw a full panel;
        // TODO: allow widgets to mark when they have translucent areas and
        //       should therefore skip this test?
        if (!m_widget->shape().contains(m_widget->contentsRect())) {
            QPainterPath p;
            p.addRect(m_widget->boundingRect());
            if (m_widget->shape() != p) {
                addedMargin = m_widget->contentsRect().width() / 2;
            }
        }

        if (m_buttonsOnRight) {
            marginLeft += addedMargin;
        } else {
            marginRight += addedMargin;
        }
    }

    //m_rect = m_widget->mapToParent(m_rect).boundingRect();
    m_decorationRect = m_rect.adjusted(-marginLeft, -marginTop, marginRight, marginBottom);
    m_totalRect = m_decorationRect.united(m_widget->boundingRect());
}

#include "handle.moc"

