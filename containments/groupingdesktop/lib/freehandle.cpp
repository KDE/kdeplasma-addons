/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#include "freehandle.h"

#include <QApplication>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QTouchEvent>
#include <QMatrix>
#include <QTransform>
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
qreal _k_distanceForPoint(const QPointF& point);
qreal _k_pointAngle(const QPointF& point);
QPointF _k_rotatePoint(const QPointF& point, qreal angle);
QPointF _k_projectPoint(QPointF point, QPointF v);

FreeHandle::FreeHandle(GroupingContainment *parent, Plasma::Applet *applet)
      : Handle(parent, applet),
        m_pressedButton(NoButton),
        m_iconSize(KIconLoader::SizeSmall),
        m_opacity(0.0),
        m_animType(FadeIn),
        m_backgroundBuffer(0),
        m_buttonsOnRight(false),
        m_pendingFade(false)
{
    init();
}

FreeHandle::FreeHandle(GroupingContainment *parent, AbstractGroup *group)
      : Handle(parent, group),
        m_pressedButton(NoButton),
        m_iconSize(KIconLoader::SizeSmall),
        m_opacity(0.0),
        m_animType(FadeIn),
        m_backgroundBuffer(0),
        m_buttonsOnRight(false),
        m_pendingFade(false)
{
    init();
}

FreeHandle::~FreeHandle()
{
    detachWidget();
    delete m_backgroundBuffer;
}

void FreeHandle::init()
{
    setFlags(flags() | QGraphicsItem::ItemStacksBehindParent);
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View,
                             Theme::defaultTheme()->colorScheme());
    setAcceptTouchEvents(true);
    m_gradientColor = colorScheme.background(KColorScheme::NormalBackground).color();
    m_originalGeom = mapToScene(QRectF(QPoint(0,0), widget()->size())).boundingRect();
    m_originalTransform = widget()->transform();

    QPointF center = QRectF(QPointF(), widget()->size()).center();
    m_angle = _k_pointAngle(m_originalTransform.map(center + QPointF(1.0, 0.0)) - center);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(333);

    m_leaveTimer = new QTimer(this);
    m_leaveTimer->setSingleShot(true);
    m_leaveTimer->setInterval(500);

    connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    connect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    connect(widget(), SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed()));

    setAcceptsHoverEvents(true);
    m_hoverTimer->start();

    //icons
    m_configureIcons = new Svg(this);
    m_configureIcons->setImagePath("widgets/configuration-icons");
    m_configureIcons->setContainsMultipleImages(true);

    m_background = new FrameSvg(this);
    m_background->setImagePath("widgets/background");
    widget()->installSceneEventFilter(this);

    calculateSize();
}

// void FreeHandle::detachWidget()
// {
//     if (!widget()) {
//         return;
//     }
//
//     disconnect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
//     disconnect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
//     widget()->disconnect(this);
//
//     if (applet() && (applet()->geometry() != m_originalGeom || applet()->transform() != m_originalTransform)) {
// //         emit applet()->appletTransformedByUser(); //FIXME: protected!
//     } else if (group() && (group()->geometry() != m_originalGeom || group()->transform() != m_originalTransform)) {
//         emit group()->groupTransformedByUser();
//     }
//
//     applet() = 0;
//     group() = 0;
//     widget() = 0;
// }

QRectF FreeHandle::boundingRect() const
{
    return m_totalRect;
}

QPainterPath FreeHandle::shape() const
{
    //when the containment changes the widget is reset to 0
    if (widget()) {
        QPainterPath path = PaintUtils::roundedRectangle(m_decorationRect, 10);
        return path.united(widget()->shape());
    } else {
        return QGraphicsItem::shape();
    }
}

QPainterPath FreeHandleRect(const QRectF &rect, int radius, bool onRight)
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

void FreeHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
                g.setColorAt(qMax(qreal(0.0), opaquePoint - qreal(0.05)), Qt::transparent);
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
        AbstractGroup *parentGroup = this->widget()->property("group").value<AbstractGroup *>();
        if (!(parentGroup && parentGroup->groupType() == AbstractGroup::ConstrainedGroup)) {
            if (m_buttonsOnRight) {
                m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tr2bl");
            } else {
                m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tl2br");
            }
            iconRect.translate(0, m_iconSize);

            m_configureIcons->paint(&buffPainter, iconRect, "rotate");
            iconRect.translate(0, m_iconSize);
        }

        if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
            m_configureIcons->paint(&buffPainter, iconRect, "configure");
            iconRect.translate(0, m_iconSize);
        }

        if (applet() && applet()->hasValidAssociatedApplication()) {
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

    AbstractGroup *parentGroup = this->widget()->property("group").value<AbstractGroup *>();
    if (!(parentGroup && parentGroup->groupType() == AbstractGroup::ConstrainedGroup)) {
        if (group() || (applet() && applet()->aspectRatioMode() != FixedSize)) {
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
    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        painter->drawPixmap(
            QRectF(basePoint + shiftC, iconSize), *m_backgroundBuffer, sourceIconRect);
        sourceIconRect.translate(0, m_iconSize);
        basePoint += step;
    }

    //maximize
    if (applet() && applet()->hasValidAssociatedApplication()) {
        painter->drawPixmap(
            QRectF(basePoint + shiftMx, iconSize), *m_backgroundBuffer, sourceIconRect);
        sourceIconRect.translate(0, m_iconSize);
        basePoint += step;
    }

    //close
    basePoint = m_rect.bottomLeft() + QPointF(HANDLE_MARGIN, 0) - step;
    painter->drawPixmap(QRectF(basePoint + shiftD, iconSize), *m_backgroundBuffer, sourceIconRect);
}

void FreeHandle::emitDisappear()
{
    emit disappearDone(this);
}

Handle::ButtonType FreeHandle::mapToButton(const QPointF &point) const
{
    int iconMargin = m_iconSize / 2;
    //XXX this code is duplicated in the prev. function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_MARGIN, iconMargin);
    QPointF step = QPointF(0, m_iconSize + iconMargin);
    QPointF separator = step + QPointF(0, iconMargin);
   //end duplicate code

    QRectF activeArea = QRectF(basePoint, QSizeF(m_iconSize, m_iconSize));

    AbstractGroup *parentGroup = widget()->property("group").value<AbstractGroup *>();
    if (!(parentGroup && parentGroup->groupType() == AbstractGroup::ConstrainedGroup)) {
        if (group() || (applet() && applet()->aspectRatioMode() != FixedSize)) {
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

    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        if (activeArea.contains(point)) {
            return ConfigureButton;
        }
        activeArea.translate(step);
    }

    if (applet() && applet()->hasValidAssociatedApplication()) {
        if (activeArea.contains(point)) {
            return MaximizeButton;
        }
    }

    activeArea.moveTop(m_rect.bottom() - activeArea.height() - iconMargin);
    if (activeArea.contains(point)) {
        return RemoveButton;
    }

    return MoveButton;
    //return applet()->mapToParent(applet()->shape()).contains(point) ? NoButton : MoveButton;
}

void FreeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //containment recently switched?
    if (!widget()) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    if (m_pendingFade) {
        //m_pendingFade = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_originalGeom = mapToScene(QRectF(QPoint(0,0), widget()->size())).boundingRect();
        QPointF center = widget()->boundingRect().center();
        m_originalTransform = widget()->transform();
        m_angle = _k_pointAngle(m_originalTransform.map(center + QPointF(1.0, 0.0)) - center);

        m_pressedButton = mapToButton(event->pos());
        //kDebug() << "button pressed:" << m_pressedButton;
        if (m_pressedButton != NoButton) {
            containment()->raise(widget());
            m_zValue = widget()->zValue();
            setZValue(m_zValue);
        }

        if (m_pressedButton == MoveButton) {
            containment()->setMovingWidget(widget());
        }

        if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
            m_originalGeom = mapToScene(QRectF(QPoint(0,0), widget()->size())).boundingRect();
            m_origWidgetCenter = m_originalGeom.center();
            m_origWidgetSize = QPointF(widget()->size().width(), widget()->size().height());

            // resize
            if (m_buttonsOnRight) {
                m_resizeStaticPoint = widget()->mapToScene(QPointF(0, widget()->size().height()));
            } else {
                m_resizeStaticPoint = widget()->mapToScene(m_origWidgetSize);
            }
            m_resizeGrabPoint = event->scenePos();

            // rotate
            m_rotateAngleOffset = m_angle - _k_pointAngle(event->scenePos() - m_origWidgetCenter);
        }

        event->accept();

        //set mousePos to the position in the widget, in screencoords, so it becomes easy
        //to reposition the toplevel view to the correct position.
        if (currentView() && widget()) {
            QPoint localpos = currentView()->mapFromScene(widget()->scenePos());
            m_mousePos = event->screenPos() - currentView()->mapToGlobal(localpos);
        }
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void FreeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug() << "button pressed:" << m_pressedButton << ", fade pending?" << m_pendingFade;

    if (m_pendingFade) {
        startFading(FadeOut, m_entryPos);
        m_pendingFade = false;
    }

    ButtonType releasedAtButton = mapToButton(event->pos());

    if (widget() && event->button() == Qt::LeftButton) {
        switch (m_pressedButton) {
        case ConfigureButton:
            //FIXME: Remove this call once the configuration management change was done
            if (applet() && m_pressedButton == releasedAtButton) {
                applet()->showConfigurationInterface();
            } else if (group() && group()->hasConfigurationInterface()) {
                group()->showConfigurationInterface();
            }
            break;
        case RemoveButton:
            if (m_pressedButton == releasedAtButton) {
                forceDisappear();
                if (applet()) {
                    applet()->destroy();
                } else {
                    group()->destroy();
                }
                return;
            }
            break;
        case MoveButton:
        {
                // test for containment change
                //kDebug() << "testing for containment change, sceneBoundingRect = "
                //         << containment()->sceneBoundingRect();
                if (!containment()->sceneBoundingRect().contains(widget()->scenePos())) {
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

                emit widgetMoved(widget());

            break;
        }
        case MaximizeButton:
            if (applet()) {
                applet()->runAssociatedApplication();
            }
            break;
        default:
            break;
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(const QPointF& point)
{
    return std::sqrt(point.x() * point.x() + point.y() * point.y());
}

qreal _k_pointAngle(const QPointF& point)
{
    qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
    qreal cosine = point.x() / r;

    if (point.y() >= 0) {
        return acos(cosine);
    } else {
        return -acos(cosine);
    }
}

QPointF _k_rotatePoint(const QPointF& point, qreal angle)
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

void FreeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = M_PI_2 /* $i 3.14159 / 2.0 */;

    if (!widget()) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    //Track how much the mouse has moved.
    QPointF deltaScene  = event->scenePos() - event->lastScenePos();

    if (m_pressedButton == MoveButton) {
        if (leaveCurrentView(event->screenPos())) {
            Plasma::View *v = Plasma::View::topLevelViewAt(event->screenPos());
            if (v && v != currentView()) {
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

        QPointF curPos = event->pos();
        QPointF lastPos = event->lastPos();

        QTransform transform = widget()->transform();
        //we need to discard translation from the transform
        QTransform t(transform.m11(), transform.m12(), transform.m21(), transform.m22(), 0, 0);
        QPointF delta = t.map(curPos - lastPos);
        widget()->moveBy(delta.x(), delta.y());
    } else if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
        QPointF cursorPoint = event->scenePos();

        // the code below will adjust these based on the type of operation
        QPointF newSize;
        QPointF newCenter;
        qreal newAngle;

        // get size limits
        QSizeF min = widget()->minimumSize();
        QSizeF max = widget()->maximumSize();

        if (min.width() < KIconLoader::SizeSmall || min.height() <  KIconLoader::SizeSmall) {
            min = widget()->effectiveSizeHint(Qt::MinimumSize);
        }

        if (max.isEmpty()) {
            max = widget()->effectiveSizeHint(Qt::MaximumSize);
        }

        // If the widget doesn't have a minimum size, calculate based on a
        // minimum content area size of 16x16 (KIconLoader::SizeSmall)
        if (min.width() < KIconLoader::SizeSmall || min.height() <  KIconLoader::SizeSmall) {
            min = widget()->boundingRect().size() - widget()->contentsRect().size();
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

            QTransform t(widget()->sceneTransform());
            //take the angle relative to the scene
            qreal angle = (t.m12() > 0 ? acos(t.m11()) : -acos(t.m11()));

            QPointF rStaticPoint = _k_rotatePoint(m_resizeStaticPoint, -angle);
            QPointF rCursorPoint = _k_rotatePoint(cursorPoint, -angle);
            QPointF rGrabPoint = _k_rotatePoint(m_resizeGrabPoint, -angle);

            if (m_buttonsOnRight) {
                newSize = m_origWidgetSize + QPointF(rCursorPoint.x() - rGrabPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            } else {
                newSize = m_origWidgetSize + QPointF(rGrabPoint.x() - rCursorPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            }

            // preserving aspect ratio?
            if (applet() && ((applet()->aspectRatioMode() != Plasma::IgnoreAspectRatio &&
                 !(event->modifiers() & Qt::ControlModifier)) ||
                 (applet()->aspectRatioMode() == Plasma::IgnoreAspectRatio &&
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
                            rStaticPoint.y() - newSize.y()/2), angle);
            } else {
                newCenter =  _k_rotatePoint(QPointF(rStaticPoint.x() - newSize.x()/2,
                            rStaticPoint.y() - newSize.y()/2), angle);
            }

            newAngle = m_angle;
        }

        // apply size
        widget()->resize(newSize.x(), newSize.y());

        // apply position, no need if we're rotating
        if (m_pressedButton != RotateButton) {
            widget()->setPos(widget()->parentItem()->mapFromScene(newCenter - newSize/2));
        }

        // apply angle
        QTransform at;
        at.translate(newSize.x()/2, newSize.y()/2);
        at.rotateRadians(newAngle);
        at.translate(-newSize.x()/2, -newSize.y()/2);
        widget()->setTransform(at);
        m_angle = newAngle;
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

bool FreeHandle::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchEnd: {
        QTransform t = widget()->transform();
        QRectF geom = widget()->geometry();
        QPointF translation(t.m31(), t.m32());
        QPointF center = geom.center();
        geom.setWidth(geom.width()*qAbs(t.m11()));
        geom.setHeight(geom.height()*qAbs(t.m22()));
        geom.moveCenter(center);

        widget()->setGeometry(geom);
        t.reset();
        t.translate(widget()->size().width()/2, widget()->size().height()/2);
        t.rotateRadians(m_angle);
        t.translate(-widget()->size().width()/2, -widget()->size().height()/2);


        widget()->setTransform(t);
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
            QTransform t = widget()->transform();
            t.translate(widget()->size().width()/2, widget()->size().height()/2);
            t.rotate(line1.angleTo(line0));

            //scaling
            qreal scaleFactor = 1;
            if (line0.length() > 0) {
                scaleFactor = line1.length() / line0.length();
            }

            t.scale(scaleFactor, scaleFactor);
            t.translate(-widget()->size().width()/2, -widget()->size().height()/2);
            widget()->setTransform(t);

        }
        return true;
    }
    default:
        break;
    }
    return QGraphicsItem::sceneEvent(event);
}

void FreeHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
//     kDebug() << "hover enter";

    //if a disappear was scheduled stop the timer
    if (m_leaveTimer->isActive()) {
        m_leaveTimer->stop();
    }
    // if we're already fading out, fade back in
    else if (!m_anim.data() && m_animType == FadeOut) {
        startFading(FadeIn, m_entryPos, true);
    }
}

void FreeHandle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEnterEvent(event);
}

void FreeHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
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

bool FreeHandle::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == widget() && event->type() == QEvent::GraphicsSceneHoverLeave) {
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent*>(event));
    }

    return false;
}

void FreeHandle::setFadeAnimation(qreal progress)
{
    m_opacity = progress;
    //kDebug() << "progress" << progress << "m_opacity" << m_opacity << m_anim << "(" << FadeIn << ")";
    if (qFuzzyCompare(progress, qreal(1.0))) {
        delete m_backgroundBuffer;
        m_backgroundBuffer = 0;
    }

    update();
}

qreal FreeHandle::fadeAnimation() const
{
    return m_opacity;
}

void FreeHandle::hoverTimeout()
{
    startFading(FadeIn, m_entryPos);
}

void FreeHandle::leaveTimeout()
{
    if (!isUnderMouse()) {
        startFading(FadeOut, m_entryPos);
    }
}

void FreeHandle::widgetResized()
{
    prepareGeometryChange();
    calculateSize();
    update();
}

void FreeHandle::setHoverPos(const QPointF &hoverPos)
{
    m_entryPos = hoverPos;

    if (!boundingRect().contains(hoverPos)) {
        m_leaveTimer->start();
    }
}

void FreeHandle::startFading(FadeType anim, const QPointF &hoverPos, bool preserveSide)
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

    if (!widget()) {
        m_animType = FadeOut;
        setFadeAnimation(1.0);
        return;
    }

    if (anim == FadeIn) {
        //kDebug() << m_entryPos.x() << widget()->pos().x();
        prepareGeometryChange();
        bool wasOnRight = m_buttonsOnRight;
        if (!preserveSide) {
            m_buttonsOnRight = m_entryPos.x() > (widget()->size().width() / 2);
        }
        calculateSize();
        QPolygonF region = widget()->mapToParent(m_rect).intersected(widget()->parentWidget()->boundingRect());
        //kDebug() << region << m_rect << mapToParent(m_rect) << containmnet->boundingRect();
        if (region != widget()->mapToParent(m_rect)) {
            // switch sides
            //kDebug() << "switch sides";
            m_buttonsOnRight = !m_buttonsOnRight;
            calculateSize();
            QPolygonF region2 = widget()->mapToParent(m_rect).intersected(widget()->parentWidget()->boundingRect());
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

void FreeHandle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut, m_entryPos);
}

int FreeHandle::minimumHeight()
{
    int iconMargin = m_iconSize / 2;
    int requiredHeight =  iconMargin  + //first margin
                          (m_iconSize + iconMargin) * 4 + //XXX remember to update this if the number of buttons changes
                          iconMargin ;  //blank space before the close button

    if (applet() && applet()->hasConfigurationInterface()) {
        requiredHeight += (m_iconSize + iconMargin);
    }

    return requiredHeight;
}

void FreeHandle::calculateSize()
{
    KIconLoader *iconLoader = KIconLoader::global();
    //m_iconSize = iconLoader->currentSize(KIconLoader::Small); //does not work with double sized icon
    m_iconSize = iconLoader->loadIcon("transform-scale", KIconLoader::Small).width(); //workaround

    int handleHeight = qMax(minimumHeight(), int(widget()->contentsRect().height() * 0.8));
    int handleWidth = m_iconSize + 2 * HANDLE_MARGIN;
    int top =
        widget()->contentsRect().top() + (widget()->contentsRect().height() - handleHeight) / 2.0;

    qreal marginLeft, marginTop, marginRight, marginBottom;
    m_background->getMargins(marginLeft, marginTop, marginRight, marginBottom);

    if (m_buttonsOnRight) {
        //put the rect on the right of the widget
        m_rect = QRectF(widget()->size().width(), top, handleWidth, handleHeight);
    } else {
        //put the rect on the left of the widget
        m_rect = QRectF(-handleWidth, top, handleWidth, handleHeight);
    }

    if (widget()->contentsRect().height() > qreal(minimumHeight()) * 1.25) {
        int addedMargin = marginLeft / 2;

        // now we check to see if the shape is smaller than the contents,
        // and that the shape is not just the bounding rect; in those cases
        // we have a shaped guy and we draw a full panel;
        // TODO: allow widgets to mark when they have translucent areas and
        //       should therefore skip this test?
        if (!widget()->shape().contains(widget()->contentsRect())) {
            QPainterPath p;
            p.addRect(widget()->boundingRect());
            if (widget()->shape() != p) {
                addedMargin = widget()->contentsRect().width() / 2;
            }
        }

        if (m_buttonsOnRight) {
            marginLeft += addedMargin;
        } else {
            marginRight += addedMargin;
        }
    }

    //m_rect = widget()->mapToParent(m_rect).boundingRect();
    m_decorationRect = m_rect.adjusted(-marginLeft, -marginTop, marginRight, marginBottom);
    m_totalRect = m_decorationRect.united(widget()->boundingRect());
}

#include "freehandle.moc"

