/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef KONSOLEPROFILESAPPLET_H
#define KONSOLEPROFILESAPPLET_H

#include <Plasma/PopupApplet>

class QTreeView;
class QGraphicsProxyWidget;
class QStandardItemModel;
class QModelIndex;
class QGraphicsLinearLayout;

class KonsoleProfilesApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    KonsoleProfilesApplet(QObject *parent, const QVariantList &args);
    ~KonsoleProfilesApplet();

    QWidget *widget();
    enum SpecificRoles {
        ProfilesName = Qt::UserRole+1
    };

protected slots:
    void slotOnItemClicked(const QModelIndex &index);
    void slotUpdateSessionMenu();

protected:
    void initSessionFiles();
    void init();
private:
    QTreeView *m_listView;
    QStandardItemModel *m_konsoleModel;
};

K_EXPORT_PLASMA_APPLET(konsoleprofilesapplet, KonsoleProfilesApplet )

#endif
