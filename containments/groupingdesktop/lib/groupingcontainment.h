/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef GROUPINGCONTAINMENT_H
#define GROUPINGCONTAINMENT_H

#include <Plasma/Containment>

class AbstractGroup;
class GroupingContainmentPrivate;

class PLASMA_EXPORT GroupingContainment: public Plasma::Containment
{
    Q_OBJECT
    public:
        GroupingContainment(QObject *parent, const QVariantList &args);
        virtual ~GroupingContainment();

        void init();
        virtual void save(KConfigGroup &group) const;
        virtual void restore(KConfigGroup &group);
        QList<QAction *> contextualActions();
        AbstractGroup *addGroup(const QString &plugin, const QPointF &pos = QPointF(0, 0), int id = 0);
        void addGroup(AbstractGroup *group, const QPointF &pos);
        QList<AbstractGroup *> groups() const;

        /**
        * Call this function when an applet or a group is being moved by the user.
        * @param widget the widget to be monitored
        **/
        void setMovingWidget(QGraphicsWidget *widget);

        void raise(QGraphicsWidget *widget);

    protected:
        virtual void constraintsEvent(Plasma::Constraints constraints);
        virtual bool eventFilter(QObject *obj, QEvent *event);
        virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void saveContents(KConfigGroup &group) const;
        virtual void restoreContents(KConfigGroup &group);
        void useMainGroup(const QString &name);
        void setMainGroup(AbstractGroup *group);
        AbstractGroup *mainGroup() const;

    signals:
        void groupAdded(AbstractGroup *group, const QPointF &pos);
        void groupRemoved(AbstractGroup *group);
        void widgetStartsMoving(QGraphicsWidget *widget);

    private:
        Q_PRIVATE_SLOT(d, void handleDisappeared(Handle *handle))
        Q_PRIVATE_SLOT(d, void onGroupRemoved(AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onAppletRemoved(Plasma::Applet *applet))
        Q_PRIVATE_SLOT(d, void manageApplet(Plasma::Applet *applet, const QPointF &pos))
        Q_PRIVATE_SLOT(d, void newGroupClicked(QAction *action))
        Q_PRIVATE_SLOT(d, void deleteGroup())
        Q_PRIVATE_SLOT(d, void configureGroup())
        Q_PRIVATE_SLOT(d, void onAppletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onSubGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onWidgetMoved(QGraphicsWidget *widget))
        Q_PRIVATE_SLOT(d, void groupAppearAnimationComplete());
        Q_PRIVATE_SLOT(d, void onImmutabilityChanged(Plasma::ImmutabilityType immutability));

        GroupingContainmentPrivate *const d;

        friend class GroupingContainmentPrivate;
};

#endif // GROUPINGCONTAINMENT_H
