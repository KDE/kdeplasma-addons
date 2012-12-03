#ifndef SCROLLER_CPP
#define SCROLLER_CPP
/*
 *   Copyright 2007, 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "scroller.h"
#include "single-feed.h"
#include "feeddata.h"

#include <kdebug.h>
#include <kicon.h>
#include <ktoolinvocation.h>
#include <QGraphicsItem>
#include <QDateTime>
#include <QTimer>
#include <QList>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <plasma/widgets/iconwidget.h>
#define BTN_SIZE 20

using namespace Plasma;

Scroller::Scroller(QGraphicsItem *parent) :
        QGraphicsWidget(parent),
        m_current(0),
        m_animations(true),
        m_delayedNext(0),
        m_delayedPrev(0),
        m_maxAge(0),
        m_droptarget(false),
        m_left(new Plasma::IconWidget(this)),
        m_right(new Plasma::IconWidget(this)),
        m_isAnimating(false)
{

    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    m_hovered = false;

    m_left->setSvg("rssnow/arrows", "left");
    m_left->setZValue(1000);
    m_left->resize(m_left->sizeFromIconSize(10));
    m_left->hide();

    m_right->setSvg("rssnow/arrows", "right");
    m_right->setZValue(1000);
    m_right->resize(m_right->sizeFromIconSize(10));
    m_right->hide();

    m_feedIcons["generic"] = new KIcon("application-rss+xml");

    connect(m_left, SIGNAL(clicked()), this, SLOT(leftClicked()));
    connect(m_right, SIGNAL(clicked()), this, SLOT(rightClicked()));
}

Scroller::~Scroller()
{
    qDeleteAll(m_feedIcons);
}

QList<FeedData> Scroller::list()
{
    return m_list;
}

bool Scroller::hovering() const
{
    return m_hovered;
}

bool Scroller::dropTarget() const
{
    return m_droptarget;
}

void Scroller::setDropTarget(bool droptarget)
{
    m_droptarget = droptarget;
    updateGeometry();
}

void Scroller::listUpdated()
{
    if (m_droptarget && m_list.isEmpty()) {
        FeedData data;
        data.title = i18n("Drop a feed here...");
        data.extrainfo = i18n("Drop a feed here...");
        //TODO: the wording of this phrase could be better:
        data.text = i18n("...to start a new group or drop a feed on an existing group to add the feed there");
        data.icon = m_feedIcons["generic"];
        m_list.append(data);
    }

    if (m_list.isEmpty()) { //the item is fetching feeds atm
        FeedData data;
        data.title = i18n("Fetching feeds");
        data.extrainfo = i18n("Fetching feeds");
        data.icon = m_feedIcons["generic"];
        m_list.append(data);
    }

    if (m_current > (m_list.size() - 1) && !m_list.isEmpty()) {
        //feed has grown smaller, just display the first item.
        //kDebug() << "feed has grown smaller";
        m_current = 0;
    }

    //TODO: it would be neat if the actual contents of the feeditems
    //was checked so after an update the same item is displayed.
    if (m_itemlist.size() < 1) {
        SingleFeedItem * item = new SingleFeedItem(this);
        item->setFeedData(m_list.at(m_current));
        item->setZValue(0);
        item->setPos(0, 0);
        item->show();
        m_itemlist.append(item);
        m_activeitemlist.append(item);
    } else {
        m_itemlist.at(m_itemlist.size() - 1)->setFeedData(m_list.at(m_current));
    }

    if (m_droptarget) {
        updateGeometry();
    }
}

void Scroller::delayedMoveNext(int delay)
{
    QTimer::singleShot(delay, this, SLOT(moveNext()));
}

void Scroller::movePrev()
{
    if (m_isAnimating) {
        m_delayedPrev++;
    } else {
        if (m_current > 0) {
            m_current--;
        } else {
            m_current = m_list.size() - 1;
        }
        doAnimation(QAbstractAnimation::Backward);
    }
}

void Scroller::moveNext()
{
    if (m_isAnimating) {
        m_delayedNext++;
    } else {
        if (m_current < (m_list.size()-1)) {
            m_current++;
        } else {
            m_current = 0;
        }
        doAnimation(QAbstractAnimation::Forward);
    }
}

QSizeF Scroller::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which ==  Qt::MinimumSize) {
        return QSizeF(100, 32); 
    } else  if (which == Qt::PreferredSize) {
        if (m_droptarget && !m_itemlist.isEmpty()) {
            SingleFeedItem *item = m_itemlist.at(0);
            const int w = constraint.width() < 1 ? (size().width() > 0 ? size().width() : 300) : constraint.width();
            QSizeF s(w, item->preferredHeight(w));
            return s;
        } else {
            return QSizeF(300, 56);
        }
    } else if (which == Qt::MaximumSize) {
        if (m_droptarget) {
            return QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        } else {
            return QSizeF(600, 64);
        }

    }

    return QGraphicsWidget::sizeHint(which, constraint);
}

void Scroller::doAnimation(QAbstractAnimation::Direction direction)
{
    if (m_list.size() > 1) {
        if (m_animations && !m_isAnimating) {
            SingleFeedItem * item = new SingleFeedItem(this);
            item->setFeedData(m_list.at(m_current));
            item->setDisplayExtra(m_hovered);
            item->setZValue(m_itemlist.size() + 1);
            item->show();
            item->setPos((direction == QAbstractAnimation::Forward? 1 : -1) * size().width(), 0);
            item->setRect(QRect(0, 0, size().width(), size().height()));
            if (!m_itemlist.contains(item)) {
                m_itemlist.append(item);
                m_activeitemlist.append(item);
            }
            //acellerate while scrolling:
            int frames, time, queuelength;
            if (m_delayedNext > 0) {
                queuelength = m_delayedNext + 1;
            } else {
                queuelength = m_delayedPrev + 1;
            }
            time = 400 / queuelength;
            frames = time / 40;

            QPropertyAnimation *animation = m_animation.data();
            if (!animation) {
                animation = new QPropertyAnimation(this, "animate");
                animation->setStartValue(0.0);
                animation->setEndValue(1.0);
                animation->setEasingCurve(QEasingCurve::InOutQuad);
                m_animation = animation;
                connect(animation, SIGNAL(finished()), this, SLOT(animationComplete()));
            } else if (animation->state() == QAbstractAnimation::Running) {
                animation->pause();
            }

            animation->setDuration(time);
            animation->setDirection(direction);
            if (m_delayedNext > 0 || m_delayedPrev > 0) {
                animation->start(QAbstractAnimation::KeepWhenStopped);
            } else {
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            }
            m_isAnimating = true;
        } else {
            m_itemlist.at(m_itemlist.size() - 1)->setFeedData(m_list.at(m_current));
        }
    }
}

void Scroller::animationComplete()
{
    m_isAnimating = false;

    m_activeitemlist.takeFirst(); //remove the first item.
    if (m_delayedNext > 0) {
        m_delayedPrev = 0;
        m_delayedNext--;
        QTimer::singleShot(50, this, SLOT(moveNext()));
    } else if (m_delayedPrev > 0) {
        m_delayedNext = 0;
        m_delayedPrev--;
        QTimer::singleShot(50, this, SLOT(movePrev()));
    } else if (m_itemlist.size() > 2) {
        QTimer::singleShot(0, this, SLOT(clearUnusedItems()));
    }
}

qreal Scroller::animValue() const
{
    qreal xPosition = 0.0;

    foreach (SingleFeedItem *item, m_activeitemlist) {
        if (m_current == item->itemNumber()) {
            xPosition = item->pos().x();
            break;
        }
    }

    return xPosition;
}

void Scroller::animate(qreal value)
{
    if (m_animation.isNull()) {
        return;
    }

    const int width = size().width();
    const bool isForward = m_animation.data()->direction() == QAbstractAnimation::Forward;
    int xPosition;

    foreach (SingleFeedItem * item, m_activeitemlist) {
        if (m_current == item->itemNumber()) {
            xPosition = (isForward ? 1 * (1-value) : -1 * value) * width;
        } else {
            xPosition = (1-value) * width + (isForward ? -1 * width : 0);
        }
        item->setX(xPosition);
    }
}

void Scroller::clearUnusedItems()
{
    //Why is this so heavy on the resources? It makes all animations stutter.
    //So let's make sure we don't this this while animations are happening.
    foreach (SingleFeedItem * item, m_itemlist) {
        if (!m_activeitemlist.contains(item)) {
            m_itemlist.removeAll(item);
            delete item;
        }
    }
    m_itemlist.last()->setZValue(0);
}

void Scroller::setGeometry(const QRectF &geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    updateSize();
}

void Scroller::updateSize()
{
    qreal width = geometry().width();
    qreal height = geometry().height();
    QRect rect(0, 0, width, height);

    if (!m_isAnimating) {
        foreach (SingleFeedItem *item, m_itemlist) {
            item->setRect(rect);
        }
    }

    if (m_left != 0) {
        m_left->setPos(width - m_left->geometry().width(),
                       height - m_left->geometry().height());
    }

    if (m_right != 0) {
        m_right->setPos(width - m_right->geometry().width(), 0);
    }
}

void Scroller::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (event->delta() < 0) {
        moveNext();
    } else {
        movePrev();
    }
}

void Scroller::leftClicked()
{
    movePrev();
}

void Scroller::rightClicked()
{
    moveNext();
}

void Scroller::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void Scroller::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    QString url;
    if (m_current < m_list.size()) {
        url = m_list.at(m_current).url;
    }

    KToolInvocation::invokeBrowser(url);
}

void Scroller::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    if (m_list.size() > 1) {
        m_left->show();
        m_right->show();
    }
    m_hovered = true;
    foreach (SingleFeedItem * item, m_activeitemlist) {
        item->setDisplayExtra(true);
        item->update();
    }
    update();
}

void Scroller::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    if (m_list.size() > 1) {
        m_left->hide();
        m_right->hide();
    }
    m_hovered = false;
    foreach (SingleFeedItem * item, m_activeitemlist) {
        item->setDisplayExtra(false);
        item->update();
    }
    update();
}

void Scroller::setAnimations(bool animations)
{
    m_animations = animations;
}

bool Scroller::animations() const
{
    return m_animations;
}

void Scroller::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (!data.isEmpty()) {
        m_list.clear();

        //kDebug() << "feed " << source << " updated.";
        QVariantList items = data["items"].toList();

        foreach (const QVariant &tmp, items) {
            QMap<QString, QVariant> item = tmp.toMap();
            QString title = item["title"].toString();
            QString url = item["link"].toString();
            QString iconName = item["icon"].toString();
            QString feedtitle = item["feed_title"].toString();
            QString feedurl = item["feed_url"].toString();

            uint timestamp = item["time"].toUInt();
            QDateTime datetime;
            datetime.setTime_t(timestamp);

            if (!(timestamp > 0 &&  m_maxAge > 0 &&  //Only add if not too old
                datetime.addSecs(m_maxAge * 3600) < QDateTime::currentDateTime())) {
                FeedData data;
                data.text = title;
                data.title = feedtitle;
                data.url = url;
                data.time = timestamp;

                KIcon *icon = m_feedIcons.value(iconName);
                if (!icon) {
                    QPixmap p = QPixmap(iconName);
                    if (!p.isNull()) {
                        icon = new KIcon(p.scaled(16, 16));
                        m_feedIcons.insert(iconName, icon);
                    }
                }

                if (icon) {
                    data.icon = icon;
                } else {
                    data.icon = m_feedIcons["generic"];
                }

                data.itemNumber = m_list.size();
                m_list.append(data);
            }
        }

        //TODO: separate this and use a timer to update fuzzyDate
        //every minute.
        const int listSize = m_list.size();
        for (int i = 0; i < listSize; ++i) {
            uint timestamp = m_list.at(i).time;
            if (timestamp != 0) {
                QDateTime datetime;
                datetime.setTime_t(timestamp);
                m_list[i].extrainfo = QString("(%1/%2) %3")
                                                 .arg(i+1)
                                                 .arg(m_list.size())
                                                 .arg(fuzzyDate(datetime));
            } else {
                m_list[i].extrainfo = QString("(%1/%2) %3")
                                                 .arg(i+1)
                                                 .arg(m_list.size())
                                                 .arg(m_list.at(i).title);
            }
            m_list[i].title = QString("(%1/%2) %3")
                                         .arg(i+1)
                                         .arg(m_list.size())
                                         .arg(m_list.at(i).title);
        }

        if (m_list.size() < 1) {
            FeedData noitems;
            noitems.title = "no items to display";
            noitems.extrainfo = "no items to display";
            noitems.text = data["title"].toString();
            noitems.icon = m_feedIcons["generic"];

            m_list.append(noitems);
        }

        listUpdated();
    }
}

//TODO: hmm, we probably want to update the timestamps more often then the feeds
//get updated... like, every minute
QString Scroller::fuzzyDate(const QDateTime& datetime)
{
    QDateTime now = QDateTime::currentDateTime();
    QDate nowdate = now.date();
    QDate date = datetime.date();

    if (now < (datetime.addSecs(3600))) {
        return i18np( "%1 minute ago", "%1 minutes ago", (datetime.secsTo(now)/60) );
    } else if (nowdate == date.addDays(1)) {
        return i18n( "yesterday" );
    } else if (now < (datetime.addDays(1))) {
        return i18np( "%1 hour ago", "%1 hours ago", (datetime.secsTo(now)/3600) );
    } else if (now < (datetime.addDays(7))) {
        return i18np( "%1 day ago", "%1 days ago", datetime.daysTo(now));
    } else {
        return i18np( "%1 week ago", "%1 weeks ago", (datetime.daysTo(now)/7));
    }
}

int Scroller::maxAge() const
{
    return m_maxAge;
}

void Scroller::setMaxAge(int maxAge)
{
    m_maxAge = maxAge;
}


#include "scroller.moc"
#endif // SCROLLER_CPP

