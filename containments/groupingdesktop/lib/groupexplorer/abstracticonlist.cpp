/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
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

/**
 * Taken from kdebase/workspace/libs/plasmagenericshell
 */

#include "abstracticonlist.h"

#include <cmath>

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QHash>
#include <QToolButton>

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

const int ICON_SIZE = 70;
const int SEARCH_DELAY = 300;
const int SCROLL_STEP_DURATION = 300;

namespace Plasma
{

AbstractIconList::AbstractIconList(Plasma::Location loc, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_arrowsSvg(new Plasma::Svg(this)),
      m_searchDelayTimer(new QTimer(this)),
      m_iconSize(16),
      m_scrollingDueToWheel(false)
{
    m_scrollStep = 0;
    m_firstItemIndex = 0;
    m_selectedItem = 0;
    m_location = loc;
    m_orientation = ((loc == Plasma::LeftEdge || loc == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);

    //timer stuff
    m_searchDelayTimer->setSingleShot(true);
    m_searchDelayTimer->setInterval(SEARCH_DELAY);
    connect(m_searchDelayTimer, SIGNAL(timeout()), this, SLOT(setSearch()));

    // init svg objects
    m_arrowsSvg->setImagePath("widgets/arrows");
    m_arrowsSvg->setContainsMultipleImages(true);
    m_arrowsSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    m_slide = Plasma::Animator::create(Plasma::Animator::SlideAnimation);
    m_slide->setEasingCurve(QEasingCurve::Linear);
    connect(m_slide, SIGNAL(finished()), this, SLOT(scrollStepFinished()));

    init();
}

AbstractIconList::~AbstractIconList()
{

    //FIXME: if the follow foreach looks silly, that's because it is.
    //       but Qt 4.6 currently has a devastating bug that crashes
    //       when we don't do precisely this
    foreach (QGraphicsWidget *item, m_allAppletsHash) {
        item->setParentItem(0);
        item->deleteLater();
    }

    delete m_slide;
}

void AbstractIconList::init()
{
    //init arrows
    m_upLeftArrow = new Plasma::ToolButton(this);
    m_upLeftArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_upLeftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_downRightArrow = new Plasma::ToolButton(this);
    m_downRightArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_downRightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    if (m_orientation == Qt::Horizontal) {
        m_upLeftArrow->setIcon(KIcon(m_arrowsSvg->pixmap("left-arrow")));
        m_downRightArrow->setIcon(KIcon(m_arrowsSvg->pixmap("right-arrow")));
        m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
        m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
    } else {
        m_upLeftArrow->setIcon(KIcon(m_arrowsSvg->pixmap("up-arrow")));
        m_downRightArrow->setIcon(KIcon(m_arrowsSvg->pixmap("down-arrow")));
        m_upLeftArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
        m_downRightArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
    }

    connect(m_downRightArrow, SIGNAL(pressed()), this, SLOT(scrollDownRight()));
    connect(m_upLeftArrow, SIGNAL(pressed()), this, SLOT(scrollUpLeft()));

    //XXX rename later
    //init window that shows the applets of the list - it clips the appletsListWidget
    m_appletListWindowWidget = new QGraphicsWidget(this);
    m_appletListWindowWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_appletListWindowWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //init applets list
    m_appletListWidget = new QGraphicsWidget(m_appletListWindowWidget);
    m_appletListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation, m_appletListWidget);
    m_appletListLinearLayout->setSpacing(0);

    m_slide->setTargetWidget(m_appletListWidget);

    //make its events pass through its parent
    m_appletListWidget->installEventFilter(this);
    m_appletListWindowWidget->installEventFilter(this);

    //layouts
    m_arrowsLayout = new QGraphicsLinearLayout(m_orientation);

    m_arrowsLayout->addItem(m_appletListWindowWidget);
    m_arrowsLayout->addItem(m_upLeftArrow);
    m_arrowsLayout->addItem(m_downRightArrow);

    m_arrowsLayout->setAlignment(m_downRightArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_upLeftArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_appletListWindowWidget, Qt::AlignVCenter | Qt::AlignHCenter);

    //m_hoverIndicator = new Plasma::ItemBackground(m_appletListWindowWidget);
    m_hoverIndicator = new Plasma::ItemBackground(m_appletListWidget);
    setLayout(m_arrowsLayout);
}


//parent intercepts children events
bool AbstractIconList::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(obj);

        if (widget == m_appletListWidget) {
            //the resize occurred with the list widget
            if (m_orientation == Qt::Horizontal) {
                m_appletListWindowWidget->setMinimumSize(0, m_appletListWidget->minimumHeight());
            } else {
                m_appletListWindowWidget->setMinimumSize(m_appletListWidget->minimumWidth(), 0);
            }

            manageArrows();
            return false;
        } else if (widget == m_appletListWindowWidget) {
            // the resize occurred with the window widget
            // FIXME rename this too, eew.
            int maxVisibleIconsOnList = maximumAproxVisibleIconsOnList();
            m_scrollStep = ceil((float)maxVisibleIconsOnList/2);
            if (m_orientation == Qt::Vertical) {
                m_appletListWidget->setMinimumSize(m_appletListWindowWidget->size().width(), 0);
            }
            return false;
        }
    }

    return QObject::eventFilter(obj, event);
}

Plasma::Location AbstractIconList::location()
{
    return m_location;
}

void AbstractIconList::setLocation(Plasma::Location location)
{
    if (m_location == location) {
        return;
    }

    m_orientation = ((location == Plasma::LeftEdge || location == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);

    m_appletListLinearLayout->invalidate();
    m_appletListLinearLayout->setOrientation(m_orientation);
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

    m_appletListLinearLayout->activate();
}

void AbstractIconList::searchTermChanged(const QString &text)
{
    m_searchString = text;
    m_searchDelayTimer->start();
}

void AbstractIconList::setSearch()
{
    //pass it down to the subclass
    setSearch(m_searchString);
    updateList();
}

void AbstractIconList::iconHoverEnter(Plasma::AbstractIcon *icon)
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

//all items are always in the list. filter updates just hide/show.
//what do we need the visible-list for? getting position, finding the end of the list,
//calculating how many fit on the list...
//TODO it might be a bit easier if we explicitly told all icons to be the same (set) size.
void AbstractIconList::showIcon(AbstractIcon *icon)
{
    if (icon) {
        icon->expand();
        m_currentAppearingAppletsOnList.append(icon);
    }
}

//FIXME this isn't ideal. it'd be nicer if I could just use setVisible
//but I haven't eliminated the need for the visible-list yet.
void AbstractIconList::hideIcon(AbstractIcon *icon)
{
    if (icon) {
        icon->collapse();
    }

    m_currentAppearingAppletsOnList.removeAll(icon);
}

//a faster way, given that we still need the visible-list
void AbstractIconList::hideAllIcons()
{
    foreach (QWeakPointer<AbstractIcon> pIcon, m_currentAppearingAppletsOnList) {
        if (pIcon.isNull()) {
            continue;
        }
        AbstractIcon *icon = pIcon.data();
        icon->collapse();
    }
    m_currentAppearingAppletsOnList.clear();
}

//uses the average icon size to guess how many will fit
int AbstractIconList::maximumAproxVisibleIconsOnList()
{
    qreal windowSize;
    qreal listTotalSize;
    qreal iconAverageSize;
    qreal maxVisibleIconsOnList;

    if (m_orientation == Qt::Horizontal) {
        windowSize = m_appletListWindowWidget->geometry().width();
        listTotalSize = m_appletListLinearLayout->preferredSize().width();
    } else {
        windowSize = m_appletListWindowWidget->geometry().height();
        listTotalSize = m_appletListLinearLayout->preferredSize().height();
    }

    iconAverageSize = listTotalSize/
                      (m_currentAppearingAppletsOnList.count()) +
                       m_appletListLinearLayout->spacing();
//    approximatelly
    maxVisibleIconsOnList = floor(windowSize/iconAverageSize);

    return maxVisibleIconsOnList;
}

void AbstractIconList::addIcon(AbstractIcon *icon)
{
    icon->setParent(m_appletListWidget); //FIXME redundant?
    qreal l, t, r, b;
    m_hoverIndicator->getContentsMargins(&l, &t, &r, &b);
    icon->setContentsMargins(l, t, r, b);

    //we don't add it to the hash here because we don't know its id.

    //FIXME surely this should be checked in the setter :P
    if (m_iconSize != AbstractIcon::DEFAULT_ICON_SIZE) {
        icon->setIconSize(m_iconSize);
    }

    m_appletListLinearLayout->addItem(icon);
    m_appletListLinearLayout->setAlignment(icon, Qt::AlignHCenter);
    showIcon(icon);

    connect(icon, SIGNAL(hoverEnter(Plasma::AbstractIcon*)), this, SLOT(iconHoverEnter(Plasma::AbstractIcon*)));
    connect(icon, SIGNAL(clicked(Plasma::AbstractIcon*)), this, SLOT(itemSelected(Plasma::AbstractIcon*)));
}

void AbstractIconList::itemSelected(Plasma::AbstractIcon *icon)
{
    if (m_selectedItem) {
        m_selectedItem->setSelected(false);
    }

    icon->setSelected(true);
    m_selectedItem = icon;
}

void AbstractIconList::updateList()
{
    //pure virtual
    updateVisibleIcons();

    m_appletListWidget->adjustSize();

    updateGeometry();
    m_hoverIndicator->hide();
    resetScroll();
}

void AbstractIconList::wheelEvent(QGraphicsSceneWheelEvent *event)
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

void AbstractIconList::scrollDownRight()
{
    int nextFirstItemIndex = m_firstItemIndex + m_scrollStep;
    if (nextFirstItemIndex > m_currentAppearingAppletsOnList.count() - 1) {
        nextFirstItemIndex = m_currentAppearingAppletsOnList.count() - 1;
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
        for (int i = 0; i < m_currentAppearingAppletsOnList.count(); i++) {
            if (itemPosition(i) >= searchPosition) {
                nextFirstItemIndex = i;
                break;
            }
        }
    }

    scrollTo(nextFirstItemIndex);
}

void AbstractIconList::scrollUpLeft()
{
    int nextFirstItemIndex = m_firstItemIndex - m_scrollStep;
    if (nextFirstItemIndex < 0) {
        nextFirstItemIndex = 0;
    }

    if (nextFirstItemIndex > m_currentAppearingAppletsOnList.count() - 1) {
        manageArrows();
        return;
    }

    scrollTo(nextFirstItemIndex);
}

void AbstractIconList::scrollTo(int index)
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

void AbstractIconList::scrollStepFinished()
{
    manageArrows();

    if (m_scrollingDueToWheel && m_hoverIndicator->isVisible() && scene()) {
        QGraphicsView *view = Plasma::viewFor(this);
        if (view) {
            AbstractIcon *icon = dynamic_cast<AbstractIcon *>(scene()->itemAt(view->mapToScene(view->mapFromGlobal(QCursor::pos()))));
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

void AbstractIconList::resetScroll()
{
    m_appletListWidget->setPos(0,0);
    m_firstItemIndex = 0;
    manageArrows();
}

void AbstractIconList::manageArrows()
{
    qreal list_size = listSize();
    qreal window_size = windowSize();

    if (list_size <= window_size || m_currentAppearingAppletsOnList.isEmpty()) {
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

QRectF AbstractIconList::visibleListRect()
{
    QRectF visibleRect = m_appletListWindowWidget->
                         mapRectToItem(m_appletListWidget, 0, 0,
                                          m_appletListWindowWidget->geometry().width(),
                                          m_appletListWindowWidget->geometry().height());

    return visibleRect;
}

qreal AbstractIconList::visibleStartPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return m_appletListWindowWidget->mapToItem(m_appletListWidget, m_appletListWindowWidget->boundingRect().left(), 0).x();
    } else {
        return m_appletListWindowWidget->mapToItem(m_appletListWidget, 0, m_appletListWindowWidget->boundingRect().top()).y();
    }
}

qreal AbstractIconList::visibleEndPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return m_appletListWindowWidget->mapToItem(m_appletListWidget, m_appletListWindowWidget->boundingRect().right(), 0).x();
    } else {
        return m_appletListWindowWidget->mapToItem(m_appletListWidget, 0, m_appletListWindowWidget->boundingRect().bottom()).y();
    }
}

qreal AbstractIconList::listSize()
{
    if (m_orientation == Qt::Horizontal) {
        return m_appletListWidget->boundingRect().size().width();
    } else {
        return m_appletListWidget->boundingRect().size().height();
    }
}

qreal AbstractIconList::windowSize()
{
    return (visibleEndPosition() - visibleStartPosition());
}

qreal AbstractIconList::itemPosition(int i)
{
    AbstractIcon *applet = m_currentAppearingAppletsOnList.value(i).data();
    if (!applet) {
        return 0;
    }

    if (m_orientation == Qt::Horizontal) {
        return applet->pos().x();
    } else {
        return applet->pos().y();
    }
}

void AbstractIconList::setIconSize(int size)
{
    if (m_iconSize == size || size < 16) {
        return;
    }

    m_iconSize = size;

    foreach (AbstractIcon *applet, m_allAppletsHash) {
        applet->setIconSize(size);
    }

    adjustSize();
}

int AbstractIconList::iconSize() const
{
    return m_iconSize;
}

} // namespace Plasma

