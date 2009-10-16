/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ABSTRACTGROUP_H
#define ABSTRACTGROUP_H

#include <QtGui/QGraphicsWidget>
#include <KDE/Plasma/Applet>

class KConfigGroup;

namespace Plasma {
    class Applet;
    class FrameSvg;
};

class AbstractGroup : public QGraphicsWidget
{
    Q_OBJECT
    public:
        AbstractGroup(int id, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
        ~AbstractGroup();

        virtual void assignApplet(Plasma::Applet *applet);
        virtual void saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) = 0;
        virtual void restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group) = 0;
        virtual QString plugin() = 0;
        int id() const;
        Plasma::Applet::List assignedApplets();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void dropEvent(QGraphicsSceneDragDropEvent *event);

        Plasma::FrameSvg *m_background;
        int m_id;
        Plasma::Applet::List m_applets;
};

#endif // ABSTRACTGROUP_H
