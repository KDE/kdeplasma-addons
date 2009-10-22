/******************************************************************************
*                                   Container                                 *
*******************************************************************************
*                                                                             *
*        Copyright (C) 2009 Giulio Camuffo <giuliocamuffo@gmail.com>          *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License along   *
*   with this program; if not, write to the Free Software Foundation, Inc.,   *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                *
*                                                                             *
******************************************************************************/

#ifndef GROUPINGDESKTOP_H
#define GROUPINGDESKTOP_H

#include <Plasma/Containment>

class AbstractGroup;
class GroupingDesktopPrivate;

class GroupingDesktop: public Plasma::Containment
{
    Q_OBJECT
    public:
        GroupingDesktop(QObject *parent, const QVariantList &args);
        ~GroupingDesktop();

        void init();

        void saveContents(KConfigGroup &group) const;
        void restoreContents(KConfigGroup& group);
        QList<QAction *> contextualActions();
        void addGroup(const QString &plugin, const QPointF &pos, int id = 0);

    protected:
//         void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    private slots:
        void onGroupAdded(AbstractGroup *group, const QPointF &pos);
        void onGroupRemoved(AbstractGroup *group);
        void layoutApplet(Plasma::Applet *applet, const QPointF &pos);
        void newGridLayoutClicked();
        void removeGroupClicked();


    private:
        GroupingDesktopPrivate *const d;

        friend class GroupingDesktopPrivate;
};

#endif