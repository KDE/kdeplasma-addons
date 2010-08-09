/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#ifndef GROUPINGPANEL_H
#define GROUPINGPANEL_H

#include "../lib/groupingcontainment.h"

class QAction;

namespace Plasma
{
    class FrameSvg;
}

class GroupingPanel : public GroupingContainment
{
    Q_OBJECT
    public:
        GroupingPanel(QObject *parent, const QVariantList &args);
        ~GroupingPanel();
        void init();
        QList<QAction*> contextualActions();

        void constraintsEvent(Plasma::Constraints constraints);

        void paintInterface(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);
        void paintBackground(QPainter *painter, const QRect &contentsRect);

    protected:
        void saveState(KConfigGroup &config) const;

    private slots:
        void themeUpdated();
        void backgroundChanged();

    private:
        /**
        * update the formfactor based on the location
        */
        void setFormFactorFromLocation(Plasma::Location loc);

        /**
        * recalculate which borders to show
        */
        void updateBorders(const QRect &geom);

        Plasma::FrameSvg *m_background;
        QAction* m_configureAction;

        //cached values
        QSize m_currentSize;
        QRect m_lastViewGeom;
        bool m_maskDirty;
};


#endif // PLASMA_PANEL_H
