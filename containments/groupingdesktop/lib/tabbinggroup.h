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

#ifndef TABBEDGROUP_H
#define TABBEDGROUP_H

#include <lib/abstractgroup.h>
#include <Plasma/TabBar>
#include <QtGui/QGraphicsLinearLayout>
#include <Plasma/PushButton>
#include <KDE/KConfigDialog>
#include "ui_tabbinggroup_config.h"

class TabbingGroup : public AbstractGroup
{
    Q_OBJECT

    public:
        explicit TabbingGroup ( QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0 );
        virtual ~TabbingGroup();
        virtual QString pluginName() const;
        virtual void restoreChildGroupInfo ( QGraphicsWidget* child, const KConfigGroup& group );
        virtual void saveChildGroupInfo ( QGraphicsWidget* child, KConfigGroup group ) const;
        void save(KConfigGroup &group) const;
        void restore(KConfigGroup &group);
        virtual void createConfigurationInterface(KConfigDialog *parent);

    protected:
        virtual void layoutChild ( QGraphicsWidget* child, const QPointF& pos );

    private slots:
        void tabbarIndexChanged ( int index );
        void onAppletAdded ( Plasma::Applet *applet, AbstractGroup * );
        void onAppletRemoved ( Plasma::Applet *applet, AbstractGroup * );
        void onSubGroupAdded ( AbstractGroup *subGroup, AbstractGroup * );
        void onSubGroupRemoved ( AbstractGroup *subGroup, AbstractGroup * );
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
        void renameTabs(QStringList titles);
        Plasma::TabBar* tabbar;
        QGraphicsLinearLayout* layout;
        QMultiMap<int, QGraphicsWidget*> m_children;
        QList<int> config_mapper;
        int current_index;
        QGraphicsWidget *tabwidget;
        Plasma::PushButton *addbutton;
        Ui_TabbingGroupConfig ui;
};

#endif // TABBEDGROUP_H
