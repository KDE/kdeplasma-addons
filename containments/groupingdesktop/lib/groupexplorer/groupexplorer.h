/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QGraphicsWidget>
#include <Plasma/Plasma>

namespace Plasma
{
class Corona;
class Containment;
}
class GroupExplorerPrivate;

class GroupExplorer : public QGraphicsWidget
{
    Q_OBJECT
    public:
        GroupExplorer(Qt::Orientation orientation, QGraphicsItem *parent=0);
        GroupExplorer(QGraphicsItem *parent=0);
        ~GroupExplorer();

        /**
        * Changes the current containment
        * you've got to call this at least once so that it can access the corona
        * FIXME if you can use scene() as corona, get rid of this
        */
        void setContainment(Plasma::Containment *containment);

        /**
        * set orientation
        */
        void setOrientation(Qt::Orientation orientation);

        /**
        * @return the orientation
        */
        Qt::Orientation orientation();

        //FIXME what logic does this iconsize thing follow?
        /**
        * Sets the icon size
        */
        void setIconSize(int size);

        /**
        * @return the icon size
        */
        int iconSize() const;

    Q_SIGNALS:
        void orientationChanged(Qt::Orientation orientation);
        void closeClicked();

    public Q_SLOTS:
        void immutabilityChanged(Plasma::ImmutabilityType type);

    protected:
        void showEvent(QShowEvent *e);

    private:
        Q_PRIVATE_SLOT(d, void containmentDestroyed())

        GroupExplorerPrivate *const d;

};

#endif
