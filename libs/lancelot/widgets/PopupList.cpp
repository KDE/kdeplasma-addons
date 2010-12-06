/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PopupList.h"
#include "PopupList_p.h"

#include <QApplication>
#include <QDesktopWidget>

#include <Plasma/Theme>

#include <KMessageBox>
// #include <KWindowSystem>

#define MINIMUM_ITEM_HEIGHT 24
#define PREFERRED_ITEM_HEIGHT 32
#define MAXIMUM_ITEM_HEIGHT 32
#define ICON_SIZE QSize(24, 24)
#define MENU_WIDTH 100
#define POP_BORDER_OFFSET 16
#define POP_MINIMUM_OFFSET 64

namespace Lancelot {

PopupListMarginCache * PopupListMarginCache::m_instance = NULL;

PopupListMarginCache::PopupListMarginCache()
    : m_width(-1), m_height(-1)
{
}

PopupListMarginCache * PopupListMarginCache::self()
{
    if (!m_instance) {
        m_instance = new PopupListMarginCache();
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            m_instance, SLOT(plasmaThemeChanged()));
    }
    return m_instance;
}

int PopupListMarginCache::width()
{
    if (m_width == -1) {
        updateSizes();
    }
    return m_width;
}

int PopupListMarginCache::height()
{
    if (m_height == -1) {
        updateSizes();
    }
    return m_height;
}

void PopupListMarginCache::plasmaThemeChanged()
{
    m_width = m_height = -1;
}

void PopupListMarginCache::updateSizes()
{
    Plasma::FrameSvg * bgsvg = new Plasma::FrameSvg(this);
    bgsvg->setImagePath("dialogs/background");

    m_width  = bgsvg->marginSize(Plasma::LeftMargin)
                + bgsvg->marginSize(Plasma::RightMargin) + 4;
    m_height = bgsvg->marginSize(Plasma::TopMargin)
                + bgsvg->marginSize(Plasma::BottomMargin) + 4;
}

PopupList::Private::Private(PopupList * parent)
    : listModel(NULL),
      treeModel(NULL),
      openAction(PopupList::PopupNew),
      closeTimeout(1000),
      child(NULL),
      parentList(NULL),
      q(parent),
      hovered(false)
{
    scene = new QGraphicsScene();
    list = new ActionListView();

    scene->addItem(list);
    parent->setGraphicsWidget(list);
    parent->resize(MENU_WIDTH, 384);

    list->setShowsExtendersOutside(false);
    list->setGroupByName("PopupList");
    list->setItemsGroup(Global::self()
         ->group("PopupList-Items"));
    list->setCategoriesGroup(Global::self()
         ->group("PopupList-Categories"));

    list->setItemHeight(MINIMUM_ITEM_HEIGHT, Qt::MinimumSize);
    list->setItemHeight(PREFERRED_ITEM_HEIGHT, Qt::PreferredSize);
    list->setItemHeight(MAXIMUM_ITEM_HEIGHT, Qt::MaximumSize);

    list->setCategoryHeight(MINIMUM_ITEM_HEIGHT, Qt::MinimumSize);
    list->setCategoryHeight(PREFERRED_ITEM_HEIGHT, Qt::PreferredSize);
    list->setCategoryHeight(MAXIMUM_ITEM_HEIGHT, Qt::MaximumSize);

    list->setItemIconSize(ICON_SIZE);
    list->setCategoryIconSize(ICON_SIZE);

}

void PopupList::Private::connectSignals()
{
    connect(list->list()->itemFactory(), SIGNAL(updated()),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemInserted(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemDeleted(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(itemAltered(int)),
            q, SLOT(updateSize()));
    connect(list->list()->itemFactory(), SIGNAL(activated(int)),
            this, SLOT(listItemActivated(int)));
}

PopupList::Private::~Private()
{
    delete list;
    delete scene;
}

void PopupList::Private::listItemActivated(int index)
{
    if (treeModel && treeModel->isCategory(index)) {
        switch (openAction) {
            case OpenInside:
                list->setModel(treeModel->child(index));
                break;
            case PopupNew:
                if (!child) {
                    child = new PopupList(q);
                }
                child->setModel(treeModel->child(index));
                child->exec(QCursor::pos(), q);
                break;
            default:
                break;
                // nothing
        }
    } else {
        hidePopupAndParents();
    }

    emit q->activated(index);
}

void PopupList::Private::hidePopupAndParents()
{
    PopupList * list = q;

    while (list) {
        list->hide();
        list = list->parentList();
    }
}

PopupList::PopupList(QWidget * parent, Qt::WindowFlags f)
  : Plasma::Dialog(parent, f),
    d(new Private(this))
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    d->list->setDisplayMode(ActionListView::SingleLineNameFirst);

    d->animation = new QPropertyAnimation(this, "pos", this);
    d->animation->setEasingCurve(QEasingCurve::InOutQuad);
    d->animation->setDuration(250);
}

PopupList::~PopupList()
{
    delete d;
}

void PopupList::setModel(ActionListModel * model)
{
    if (!model) {
        return;
    }

    if (d->child) {
        d->child->hide();
    }

    d->treeModel = dynamic_cast < ActionTreeModel * > (model);

    if (!d->treeModel) {
        d->listModel = model;
        d->list->setCategoriesGroup(Global::self()
                ->group("PopupList-CategoriesPass"));
    } else {
        d->listModel = NULL;
        d->list->setCategoriesActivable(true);
        d->list->setCategoriesGroup(Global::self()
                ->group("PopupList-CategoriesPass"));
    }

    d->list->setModel(model);
    d->connectSignals();
}

void PopupList::showEvent(QShowEvent * event)
{
    Plasma::Dialog::showEvent(event);
    d->list->setFocus();
    d->timer.stop();
}

// void PopupList::show()
// {
//     Plasma::Dialog::show();
//
//     KWindowSystem::forceActiveWindow(winId());
// }

void PopupList::hideEvent(QHideEvent * event)
{
    Plasma::Dialog::hideEvent(event);

    if (d->child) {
        d->child->hide();
    }

    if (parentList()) {
        if (!parentList()->d->hovered) {
            parentList()->close();
        }
    }
}

void PopupList::setCloseTimeout(int timeout)
{
    d->closeTimeout = timeout;
}

int PopupList::closeTimeout() const
{
    return d->closeTimeout;
}

PopupList::SublevelOpenAction PopupList::sublevelOpenAction() const
{
    return d->openAction;
}

void PopupList::setSublevelOpenAction(SublevelOpenAction action)
{
    d->openAction = action;
}

void PopupList::enterEvent(QEvent * event)
{
    d->timer.stop();
    Plasma::Dialog::enterEvent(event);

    d->hovered = true;
}

void PopupList::leaveEvent(QEvent * event)
{
    if (d->closeTimeout) {
        d->timer.start(d->closeTimeout, this);
    }
    Plasma::Dialog::leaveEvent(event);

    d->hovered = false;
}

void PopupList::timerEvent(QTimerEvent * event)
{
    if (d->timer.timerId() == event->timerId()) {
        if (!d->child || d->child->isHidden()) {
            close();

            d->timer.stop();
        }
    }
    Plasma::Dialog::timerEvent(event);
}

void PopupList::updateSize()
{
    qreal width = d->list->preferredWidth();
    if (width < MENU_WIDTH) {
        width = MENU_WIDTH;
    }

    qreal height =
            (d->list->list()->itemFactory()->itemCount()) * PREFERRED_ITEM_HEIGHT;
    d->list->resize(width, height);

    resize(width  + PopupListMarginCache::self()->width(),
           height + PopupListMarginCache::self()->height());
}

void PopupList::Private::prepareToShow()
{
    q->updateSize();
    timer.stop();
}

void PopupList::exec(const QPoint & p)
{
    d->prepareToShow();

    d->parentList = NULL;

    QRect g = geometry();
    g.moveTopLeft(p);

    QRect screen = QApplication::desktop()->screenGeometry(
            QApplication::desktop()->screenNumber(p)
        );

    if (g.right() > screen.right()) {
        g.moveRight(screen.right());
    } else if (g.left() < screen.left()) {
        g.moveLeft(screen.left());
    }

    if (g.bottom() > screen.bottom()) {
        g.moveBottom(screen.bottom());
    } else if (g.top() < screen.top()) {
        g.moveTop(screen.top());
    }

    moveTo(g.topLeft());

    show();
}

void PopupList::exec(const QPoint & p, PopupList * parent)
{
    d->prepareToShow();

    d->parentList = parent;

    QRect selfGeometry = geometry();
    selfGeometry.moveTopLeft(p);

    QRect screenGeometry = QApplication::desktop()->screenGeometry(
            QApplication::desktop()->screenNumber(p)
        );

    QRect rootParentGeometry;
    int numberOfParentLists = 0;

    PopupList * list = this;

    while (list->parentList()) {
        list = list->parentList();
        numberOfParentLists++;
    }

    rootParentGeometry = list->geometry();

    // Moving...
    selfGeometry.moveLeft(rootParentGeometry.right() - POP_BORDER_OFFSET);
    if (selfGeometry.right() > screenGeometry.right()) {
        selfGeometry.moveRight(screenGeometry.right());

        if (selfGeometry.left() - rootParentGeometry.left() <
                numberOfParentLists * POP_MINIMUM_OFFSET) {
            rootParentGeometry.moveLeft(selfGeometry.left()
                - numberOfParentLists * POP_MINIMUM_OFFSET);
        }
    }

    if (selfGeometry.bottom() > screenGeometry.bottom()) {
        selfGeometry.moveBottom(screenGeometry.bottom());
    } else if (selfGeometry.top() < screenGeometry.top()) {
        selfGeometry.moveTop(screenGeometry.top());
    }

    moveTo(selfGeometry.topLeft());

    list = this;

    int shift = (selfGeometry.left() - rootParentGeometry.left())
        / numberOfParentLists;
    int left  = selfGeometry.left();

    while (list->parentList()) {
        list = list->parentList();

        QPoint parentPosition = list->pos();

        left -= shift;
        parentPosition.setX(left);
        list->moveTo(parentPosition);

    }

    show();
}

void PopupList::moveTo(const QPoint & to)
{
    if (!isVisible() ||
            !(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        move(to);
        return;
    }

    if (d->animation->state() == QAbstractAnimation::Running) {
        d->animation->stop();
    }

    d->animation->setEndValue(to);
    d->animation->start();
}

bool PopupList::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            close();
        }
    }
    return Plasma::Dialog::eventFilter(object, event);
}

PopupList * PopupList::parentList() const
{
    return d->parentList;
}

} // namespace Lancelot

