/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <lancelot/widgets/TabBar.h>

#include <QPainter>
#include <QSignalMapper>
#include <QGraphicsSceneMoveEvent>

#include <KDebug>

#include <Plasma/FrameSvg>

#include <lancelot/widgets/ExtenderButton.h>
#include <lancelot/widgets/CustomItemBackground_p.h>
#include <lancelot/Global.h>

namespace Lancelot
{

class TabBar::Private {
public:
    Private(TabBar * parent);

    Qt::Orientation orientation;
    Qt::Orientation textDirection;
    Plasma::Flip flip;

    QString currentTab;
    QString groupName;
    QSize tabIconSize;

    QMap < QString, ExtenderButton * > tabs;
    QList < ExtenderButton * > tabButtons;

    QMap < ExtenderButton *, QPair < QString, QString > > mimes;

    QSignalMapper mapper;
    CustomItemBackground * background;

    void relayout();
    void updateOrientation();
    bool isRotated() const;

    TabBar * const q;
};

TabBar::Private::Private(TabBar * parent)
    : q(parent)
{
    orientation = Qt::Horizontal;
    textDirection = Qt::Horizontal;
    groupName = QLatin1String("TabBarButton");
    tabIconSize = QSize(32, 32);

    connect(&mapper, SIGNAL(mapped(QString)),
            parent,  SIGNAL(currentTabChanged(QString)));
    connect(&mapper, SIGNAL(mapped(QString)),
            parent,  SLOT(setCurrentTab(QString)));
}

bool TabBar::Private::isRotated() const
{
    return (orientation == Qt::Vertical && textDirection == Qt::Horizontal);
}

void TabBar::Private::relayout()
{
    int diff;
    QPointF cursor = QPointF(0, 0);
    QSizeF  size = q->size();

    if (q->size().isNull() || tabs.size() == 0) {
        return;
    }

    if (orientation == Qt::Horizontal) {
       diff = q->size().width() / tabs.size();
       size.setWidth(diff);
    } else {
       diff = q->size().height() / tabs.size();
       size.setHeight(diff);
    }

    if (isRotated()) {
        cursor.ry() += diff;
        size = QSizeF(size.height(), size.width());
    }

    bool shouldFlip =
        (orientation == Qt::Vertical && (flip & Plasma::VerticalFlip))
        ||
        (orientation == Qt::Horizontal && (flip & Plasma::HorizontalFlip));

    QListIterator < ExtenderButton * > i (tabButtons);
    if (shouldFlip) {
        i.toBack();
    }

    while (shouldFlip ? i.hasPrevious() : i.hasNext()) {
        ExtenderButton * button =
           shouldFlip ? i.previous() : i.next();

        if (isRotated()) {
            button->setRotation(-90);
        } else {
            button->setRotation(0);
        }

        button->setMaximumSize(size);
        button->setGeometry(QRectF(cursor, size));

        if (orientation == Qt::Horizontal) {
            cursor.rx() += diff;
        } else {
            cursor.ry() += diff;
        }
    }

    q->setCurrentTab(currentTab);
}

void TabBar::Private::updateOrientation()
{
    foreach(Lancelot::ExtenderButton * button, tabs) {
        button->setInnerOrientation(
            (orientation == Qt::Vertical && textDirection == Qt::Horizontal)
                ? Qt::Horizontal : Qt::Vertical
        );
    }
}

TabBar::TabBar(QGraphicsWidget * parent)
    : QGraphicsWidget(parent), d(new Private(this))
{
    d->background = new CustomItemBackground(this);
    d->background->hide();
}

TabBar::~TabBar()
{
    delete d->background;
    delete d;
}

Qt::Orientation TabBar::orientation() const
{
    return d->orientation;
}

void TabBar::setOrientation(Qt::Orientation value)
{
    d->orientation = value;
    d->updateOrientation();
}

Qt::Orientation TabBar::textDirection() const
{
    return d->textDirection;
}

void TabBar::setTextDirection(Qt::Orientation value)
{
    d->textDirection = value;
    d->updateOrientation();
}

QString TabBar::currentTab() const
{
    return d->currentTab;
}

void TabBar::setCurrentTab(const QString & current)
{
    if (!d->tabs.contains(current)) {
        d->background->hide();
        return;
    }

    d->currentTab = current;

    if (d->isRotated()) {
        QRectF g = d->tabs[current]->geometry();

        g.setSize(QSizeF(g.size().height(), g.size().width()));
        g.moveTop(g.top() - g.height());

        d->background->setTarget(g);

    } else {
        d->background->setTarget(d->tabs[current]->geometry());

    }
    d->background->show();

    emit currentTabChanged(current);
}

void TabBar::addTab(const QString & id, const QIcon & icon, const QString & title,
            const QString & mimeType, const QString & mimeData)
{
    if (d->tabs.contains(id)) {
        return;
    }

    Lancelot::ExtenderButton * button = new ExtenderButton(
        icon, title, QString::null, this);
    d->tabs[id] = button;
    d->tabButtons.append(button);

    button->installSceneEventFilter(this);

    button->setIconSize(d->tabIconSize);
    button->setGroupByName(d->groupName);

    if (!mimeType.isEmpty() && !mimeData.isEmpty()) {
        d->mimes[button] = QPair < QString, QString > (mimeType, mimeData);
    }

    connect(
        button, SIGNAL(activated()),
        &(d->mapper), SLOT(map()));
    d->mapper.setMapping(button, id);


    d->relayout();
}

void TabBar::removeTab(const QString & id)
{
    if (d->tabs.contains(id)) {
        return;
    }

    d->tabButtons.removeAll(d->tabs[id]);
    d->mimes.remove(d->tabs[id]);

    delete d->tabs[id];
    d->tabs.remove(id);

    d->relayout();
}

void TabBar::setTabsGroupName(const QString & groupName)
{
    Group * group = Global::self()->group(groupName);

    if (!group) return;

    foreach (ExtenderButton * button, d->tabs) {
        button->setGroup(group);
    }

    // d->background->setSvg(group->backgroundSvg(), "down");
    d->background->setGroup(group);
    d->background->setSvgElementPrefix("down");

    d->groupName = groupName;
}

void TabBar::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Q_UNUSED(event);
    d->relayout();
}

void TabBar::setFlip(Plasma::Flip flip)
{
    d->flip = flip;
    d->relayout();
}

Plasma::Flip TabBar::flip() const
{
    return d->flip;
}


void TabBar::setTabIconSize(const QSize & size)
{
    d->tabIconSize = size;
    foreach (ExtenderButton * button, d->tabButtons) {
        button->setIconSize(size);
    }
}

QSize TabBar::tabIconSize() const
{
    return d->tabIconSize;
}

bool TabBar::sceneEventFilter(QGraphicsItem * object, QEvent * event)
{
    if (Global::self()->immutability() != Plasma::Mutable
            || event->type() != QEvent::GraphicsSceneMouseMove) {
        return false;
    }

    ExtenderButton * button = static_cast < ExtenderButton * > (object);

    if (!button
        || !d->mimes.contains(button)
    ) {
        return false;
    }

    QMimeData * data = new QMimeData();
    data->setData(
        d->mimes[button].first,
        d->mimes[button].second.toAscii());

    QDrag * drag = new QDrag(
        static_cast < QGraphicsSceneMoveEvent * > (event)->widget());
    drag->setMimeData(data);
    drag->exec();

    return false;
}

} // namespace Lancelot

