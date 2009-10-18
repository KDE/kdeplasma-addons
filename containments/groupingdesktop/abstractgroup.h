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

#include <Plasma/Applet>

class KConfigGroup;

namespace Plasma {
    class Applet;
    class FrameSvg;
};

class AbstractGroupPrivate;

class PLASMA_EXPORT AbstractGroup : public Plasma::Applet
{
    Q_OBJECT
    public:
        AbstractGroup(QObject *parent = 0, const QVariantList &args = QVariantList());
        ~AbstractGroup();

        void init();
        void assignApplet(Plasma::Applet *applet, bool layoutApplets = true);
        virtual void saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) = 0;
        virtual void restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group) = 0;
        virtual void layoutApplet(Plasma::Applet *applet) = 0;
        void destroy();
        KConfigGroup config();
        void save(KConfigGroup &group) const;
        Plasma::Applet::List assignedApplets();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);

    private slots:
        void onAppletRemoved(Plasma::Applet *applet);

    private:
        AbstractGroupPrivate *const d;
};

#endif // ABSTRACTGROUP_H
