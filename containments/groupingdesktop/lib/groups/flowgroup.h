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

#ifndef FLOWGROUP_H
#define FLOWGROUP_H

#include "abstractgroup.h"

class QGraphicsLinearLayout;
class QGraphicsSceneResizeEvent;

namespace Plasma {
    class ScrollWidget;
    class Svg;
    class ToolButton;
}

class Spacer;

class FlowGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        explicit FlowGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~FlowGroup();

        QString pluginName() const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        bool showDropZone(const QPointF &pos);
        Handle *createHandleForChild(QGraphicsWidget *child);

        static GroupInfo groupInfo();

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        void constraintsEvent(Plasma::Constraints constraints);
        void resizeEvent(QGraphicsSceneResizeEvent *event);

    private slots:
        void appletRemoved(Plasma::Applet *applet);
        void groupRemoved(AbstractGroup *group);
        void scrollPrev();
        void scrollNext();

    private:
        void updateContents();
        void addItem(QGraphicsWidget *w, const QPointF &pos);

        QGraphicsLinearLayout *m_mainLayout;
        Plasma::Svg *m_arrows;
        Plasma::ToolButton *m_prevArrow;
        Plasma::ToolButton *m_nextArrow;
        Plasma::ScrollWidget *m_scrollWidget;
        QGraphicsWidget *m_container;
        Spacer *m_spacer;


        const int SPACING;

};

#endif // FLOWGROUP_H
