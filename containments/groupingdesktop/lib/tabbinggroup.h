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

        void init();
        QString pluginName() const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void createConfigurationInterface(KConfigDialog *parent);

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        bool eventFilter(QObject *obj, QEvent *event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

    private slots:
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void tabBarIndexChanged(int index);
        void addTab(const QString &name = QString(), int pos = -1);
        void closeTab(int index = -1);
        void configModTab();
        void configUpTab();
        void configDownTab();
        void configAccepted();
        void onAppletDestroyed(Plasma::Applet *applet);
        void onGroupDestroyed(AbstractGroup *group);
        void onImmutabilityChanged(Plasma::ImmutabilityType immutability);
        void changeTab();

    private:
        void saveTabs();
        void deleteTab(int index);

        Plasma::TabBar *m_tabBar;
        QGraphicsLinearLayout *m_layout;
        QMap<QGraphicsWidget *, int> m_children;
        QList<QGraphicsWidget *> m_tabWidgets;
        Plasma::PushButton *m_newTab;
        Plasma::PushButton *m_closeTab;
        Ui_TabbingGroupConfig m_ui;
        bool m_deletingTab;
        QTimer *m_changeTabTimer;
        int m_changingTab;
};

#endif // TABBEDGROUP_H
