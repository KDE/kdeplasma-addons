/*
*   Copyright 2010 by Christian Tacke <lordbluelight@gmx.de>
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

#ifndef TABBINGGROUP_H
#define TABBINGGROUP_H

#include <lib/abstractgroup.h>

#include "ui_tabbinggroup_config.h"

class QGraphicsLinearLayout;

class KConfigDialog;

namespace Plasma {
    class TabBar;
    class PushButton;
};

class TabbingGroup : public AbstractGroup
{
    Q_OBJECT

    public:
        explicit TabbingGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~TabbingGroup();
        QString pluginName() const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void save(KConfigGroup &group) const;
        void restore(KConfigGroup &group);
        void createConfigurationInterface(KConfigDialog *parent);

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);

    private slots:
        void tabbarIndexChanged(int index);
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);
        //void addTab();
        void callConfig();
        void configAddTab();
        void configModTab();
        void configDelTab();
        void configUpTab();
        void configDownTab();
        void configAccepted();

    private:
        void hideChildren(int index);
        void showChildren(int index);
        void renameTabs(const QStringList &titles);

        Plasma::TabBar *m_tabbar;
        QGraphicsLinearLayout *m_layout;
        QMultiMap<int, QGraphicsWidget *> m_children;
        QList<int> m_config_mapper;
        int m_current_index;
        QGraphicsWidget *m_tabwidget;
        Plasma::PushButton *m_addbutton;
        Ui_TabbingGroupConfig m_ui;
};

#endif // TABBEDGROUP_H
