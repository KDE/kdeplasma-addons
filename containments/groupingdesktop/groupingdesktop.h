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

class GroupingDesktop: public Plasma::Containment
{
    Q_OBJECT
    public:
        GroupingDesktop(QObject *parent, const QVariantList &args);
        ~GroupingDesktop();

        void init();

        QList<QAction *>  contextualActions();
        void save(KConfigGroup& group) const;
        void saveContents(KConfigGroup &group) const;
        void restore(KConfigGroup& group);

    private slots:
        void newGridLayoutClicked();
        void layoutApplet(Plasma::Applet *applet, const QPointF &pos);
        void onImmutabilityChanged(Plasma::ImmutabilityType immutability);

    private:
        AbstractGroup *createGroup(const QString &plugin, int id = 0);

        QAction *m_newGridLayout;
        QMap<int, AbstractGroup *> m_groups;
};

#endif