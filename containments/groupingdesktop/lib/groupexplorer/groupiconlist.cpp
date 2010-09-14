/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "groupiconlist.h"

#include <cmath>

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QHash>
#include <QToolButton>
#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KIconLoader>
#include <KIcon>
#include <KPushButton>

#include <Plasma/Animation>
#include <Plasma/Animator>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ItemBackground>
#include <Plasma/Theme>
#include <Plasma/ToolButton>

#include "abstractgroup.h"
#include "groupicon.h"

const int ICON_SIZE = 70;
const int SEARCH_DELAY = 300;
const int SCROLL_STEP_DURATION = 300;

GroupIconList::GroupIconList(Plasma::Location loc, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_arrowsSvg(new Plasma::Svg(this)),
      m_location(Plasma::Floating),
      m_iconSize(16),
      m_scrollingDueToWheel(false)
{
    m_scrollStep = 0;
    m_firstItemIndex = 0;

    // init svg objects
    m_arrowsSvg->setImagePath("widgets/arrows");
    m_arrowsSvg->setContainsMultipleImages(true);
    m_arrowsSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    m_slide = Plasma::Animator::create(Plasma::Animator::SlideAnimation);
    m_slide->setEasingCurve(QEasingCurve::Linear);
    connect(m_slide, SIGNAL(finished()), this, SLOT(scrollStepFinished()));

    //init arrows
    m_upLeftArrow = new Plasma::ToolButton(this);
    m_upLeftArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_upLeftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_downRightArrow = new Plasma::ToolButton(this);
    m_downRightArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_downRightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    connect(m_downRightArrow, SIGNAL(pressed()), this, SLOT(scrollDownRight()));
    connect(m_upLeftArrow, SIGNAL(pressed()), this, SLOT(scrollUpLeft()));

    m_scrollWidget = new QGraphicsWidget(this);
    m_scrollWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listWidget = new QGraphicsWidget(m_scrollWidget);
    m_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listLayout = new QGraphicsLinearLayout(m_listWidget);

    m_slide->setTargetWidget(m_listWidget);

    m_listWidget->installEventFilter(this);
    m_scrollWidget->installEventFilter(this);

    m_arrowsLayout = new QGraphicsLinearLayout();
    m_arrowsLayout->addItem(m_scrollWidget);
    m_arrowsLayout->addItem(m_upLeftArrow);
    m_arrowsLayout->addItem(m_downRightArrow);
    m_arrowsLayout->setAlignment(m_downRightArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_upLeftArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_scrollWidget, Qt::AlignVCenter | Qt::AlignHCenter);

    m_hoverIndicator = new Plasma::ItemBackground(m_listWidget);

    setLayout(m_arrowsLayout);
    setLocation(loc);

    QStringList groups = AbstractGroup::availableGroups();
    foreach (const QString &group, groups) {
        GroupIcon *icon = new GroupIcon(group);
        addIcon(icon);
    }

    updateList();
}

GroupIconList::~GroupIconList()
{
    delete m_slide;
}

bool GroupIconList::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(obj);

        if (widget == m_listWidget) {
            //the resize occurred with the list widget
            if (m_orientation == Qt::Horizontal) {
                m_scrollWidget->setMinimumSize(0, m_listWidget->minimumHeight());
            } else {
                m_scrollWidget->setMinimumSize(m_listWidget->minimumWidth(), 0);
            }

            manageArrows();
        } else if (widget == m_scrollWidget) {
            // the resize occurred with the window widget
            // FIXME rename this too, eew.
            int maxVisibleIconsOnList = maximumAproxVisibleIconsOnList();
            m_scrollStep = ceil((float)maxVisibleIconsOnList/2);
            if (m_orientation == Qt::Vertical) {
                m_listWidget->setMinimumSize(m_scrollWidget->size().width(), 0);
            }
        }
    }

    return QObject::eventFilter(obj, event);
}

Plasma::Location GroupIconList::location()
{
    return m_location;
}

void GroupIconList::setLocation(Plasma::Location location)
{
    if (m_location == location) {
        return;
    }

    m_orientation = ((location == Plasma::LeftEdge || location == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);

    m_listLayout->invalidate();
    m_listLayout->setOrientation(m_orientation);
    m_arrowsLayout->setOrientation(m_orientation);

    if(m_orientation == Qt::Horizontal) {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("left-arrow"))));
        m_downRightArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("right-arrow"))));
        m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
        m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
    } else {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("up-arrow"))));
        m_downRightArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("down-arrow"))));
        m_upLeftArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
        m_downRightArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
    }

    m_listLayout->activate();
}

void GroupIconList::iconHoverEnter(GroupIcon *icon)
{
    if (icon) {
        m_hoverIndicator->setTargetItem(icon);
        if (!m_hoverIndicator->isVisible()) {
            m_hoverIndicator->setGeometry(icon->geometry());
            m_hoverIndicator->show();
        }
    } else {
        m_hoverIndicator->hide();
    }
}

//uses the average icon size to guess how many will fit
int GroupIconList::maximumAproxVisibleIconsOnList()
{
    qreal windowSize;
    qreal listTotalSize;
    qreal iconAverageSize;
    qreal maxVisibleIconsOnList;

    if (m_orientation == Qt::Horizontal) {
        windowSize = m_scrollWidget->geometry().width();
        listTotalSize = m_listLayout->preferredSize().width();
    } else {
        windowSize = m_scrollWidget->geometry().height();
        listTotalSize = m_listLayout->preferredSize().height();
    }

    iconAverageSize = listTotalSize/
                      (m_items.count()) +
                       m_listLayout->spacing();
//    approximatelly
    maxVisibleIconsOnList = floor(windowSize/iconAverageSize);

    return maxVisibleIconsOnList;
}

void GroupIconList::addIcon(GroupIcon *icon)
{
    icon->setParent(m_listWidget);
    qreal l, t, r, b;
    m_hoverIndicator->getContentsMargins(&l, &t, &r, &b);
    icon->setContentsMargins(l, t, r, b);

    //FIXME surely this should be checked in the setter :P
    if (m_iconSize != GroupIcon::DEFAULT_ICON_SIZE) {
        icon->setIconSize(m_iconSize);
    }

    m_listLayout->addItem(icon);
    m_listLayout->setAlignment(icon, Qt::AlignHCenter);
    icon->show();
    m_items.append(icon);

    connect(icon, SIGNAL(hoverEnter(GroupIcon*)), this, SLOT(iconHoverEnter(GroupIcon*)));
}

void GroupIconList::updateList()
{
    updateGeometry();
    m_hoverIndicator->hide();
    QTimer::singleShot(0, this, SLOT(resetScroll()));
}

void GroupIconList::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    bool isDownRight = event->delta() < 0;
    m_scrollingDueToWheel = true;

    if (isDownRight) {
        if (m_downRightArrow->isEnabled()) {
            scrollDownRight();
        }
    } else {
        if (m_upLeftArrow->isEnabled()) {
            scrollUpLeft();
        }
    }
}

void GroupIconList::scrollDownRight()
{
    int nextFirstItemIndex = m_firstItemIndex + m_scrollStep;
    if (nextFirstItemIndex > m_items.count() - 1) {
        nextFirstItemIndex = m_items.count() - 1;
    }

    if (nextFirstItemIndex < 0) {
        manageArrows();
        return;
    }

    qreal startPosition = itemPosition(nextFirstItemIndex);
    qreal endPosition = startPosition + (visibleEndPosition() - visibleStartPosition());

    // would we show empty space at the end?
    if (endPosition > listSize()) {
        // find a better first item
        qreal searchPosition = startPosition - (endPosition - listSize());
        for (int i = 0; i < m_items.count(); i++) {
            if (itemPosition(i) >= searchPosition) {
                nextFirstItemIndex = i;
                break;
            }
        }
    }

    scrollTo(nextFirstItemIndex);
}

void GroupIconList::scrollUpLeft()
{
    int nextFirstItemIndex = m_firstItemIndex - m_scrollStep;
    if (nextFirstItemIndex < 0) {
        nextFirstItemIndex = 0;
    }

    if (nextFirstItemIndex > m_items.count() - 1) {
        manageArrows();
        return;
    }

    scrollTo(nextFirstItemIndex);
}

void GroupIconList::scrollTo(int index)
{
    qreal startPosition = itemPosition(index);
    qreal move = startPosition - visibleStartPosition();

    m_firstItemIndex = index;

    m_slide->stop();

    if (m_orientation == Qt::Horizontal) {
        m_slide->setProperty("movementDirection", Plasma::Animation::MoveLeft);
    } else {
        m_slide->setProperty("movementDirection", Plasma::Animation::MoveUp);
    }

    m_slide->setProperty("distance", move);
    m_slide->start();

    manageArrows();
}

void GroupIconList::scrollStepFinished()
{
    manageArrows();

    if (m_scrollingDueToWheel && m_hoverIndicator->isVisible() && scene()) {
        QGraphicsView *view = Plasma::viewFor(this);
        if (view) {
            GroupIcon *icon = dynamic_cast<GroupIcon *>(scene()->itemAt(view->mapToScene(view->mapFromGlobal(QCursor::pos()))));
            if (icon) {
            //    m_hoverIndicator->setGeometry(icon->geometry());
            }
            iconHoverEnter(icon);
        }
    }
    m_scrollingDueToWheel = false;

    //keep scrolling if the button is held down
    bool movingLeftUp = m_slide->property("distance").value<qreal>() < 0;
    if (movingLeftUp) {
        if (m_upLeftArrow->isEnabled() && m_upLeftArrow->isDown()) {
            scrollUpLeft();
        }
    } else if (m_downRightArrow->isEnabled() && m_downRightArrow->isDown()) {
        scrollDownRight();
    }
}

void GroupIconList::resetScroll()
{
    m_listWidget->setPos(0,0);
    m_firstItemIndex = 0;
    manageArrows();
    m_listWidget->resize(m_listLayout->geometry().size());
}

void GroupIconList::manageArrows()
{
    qreal list_size = listSize();
    qreal window_size = windowSize();

    if (list_size <= window_size || m_items.isEmpty()) {
        m_upLeftArrow->setEnabled(false);
        m_downRightArrow->setEnabled(false);
        m_upLeftArrow->setVisible(false);
        m_downRightArrow->setVisible(false);
    } else {
        qreal endPosition = itemPosition(m_firstItemIndex) + window_size;
        m_upLeftArrow->setVisible(true);
        m_downRightArrow->setVisible(true);
        m_upLeftArrow->setEnabled(m_firstItemIndex > 0);
        m_downRightArrow->setEnabled(endPosition < list_size);
    }
}

QRectF GroupIconList::visibleListRect()
{
    QRectF visibleRect = m_scrollWidget->mapRectToItem(m_listWidget, m_scrollWidget->boundingRect());

    return visibleRect;
}

qreal GroupIconList::visibleStartPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return m_scrollWidget->mapToItem(m_listWidget, m_scrollWidget->boundingRect().left(), 0).x();
    } else {
        return m_scrollWidget->mapToItem(m_listWidget, 0, m_scrollWidget->boundingRect().top()).y();
    }
}

qreal GroupIconList::visibleEndPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return m_scrollWidget->mapToItem(m_listWidget, m_scrollWidget->boundingRect().right(), 0).x();
    } else {
        return m_scrollWidget->mapToItem(m_listWidget, 0, m_scrollWidget->boundingRect().bottom()).y();
    }
}

qreal GroupIconList::listSize()
{
    if (m_orientation == Qt::Horizontal) {
        return m_listWidget->boundingRect().size().width();
    } else {
        return m_listWidget->boundingRect().size().height();
    }
}

qreal GroupIconList::windowSize()
{
    return (visibleEndPosition() - visibleStartPosition());
}

qreal GroupIconList::itemPosition(int i)
{
    GroupIcon *applet = m_items.at(i);
    if (!applet) {
        return 0;
    }

    if (m_orientation == Qt::Horizontal) {
        return applet->pos().x();
    } else {
        return applet->pos().y();
    }
}

void GroupIconList::setIconSize(int size)
{
    if (m_iconSize == size || size < 16) {
        return;
    }

    m_iconSize = size;

    foreach (GroupIcon *applet, m_items) {
        applet->setIconSize(size);
    }

    adjustSize();
}

int GroupIconList::iconSize() const
{
    return m_iconSize;
}

#include "groupiconlist.moc"
