/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef OPENDESKTOP_H
#define OPENDESKTOP_H

// Qt
#include <QGraphicsLinearLayout>

//Plasma

#include "contactwidget.h"
#include "userwidget.h"
#include "ui_opendesktopConfig.h"
#include "ui_opendesktopLocationConfig.h"

//desktop view
namespace Plasma
{
    class DataEngine;
    class IconWidget;
    class TabBar;
    class PopupApplet;
    class ScrollWidget;
}
class KConfigDialog;

class OpenDesktop : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        OpenDesktop(QObject *parent, const QVariantList &args);
        ~OpenDesktop();
        void init();
        QGraphicsWidget* graphicsWidget();


    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private Q_SLOTS:
        void publishGeoLocation();
        void registerAccount();
        void showDetails(const Plasma::DataEngine::Data &data);
        void goHome();

    private:
        void connectGeolocation();
        void connectFriends(const QString &name);
        void connectPerson(const QString &name);
        void connectNearby(const int latitude, const int longitude);
        void disconnectPerson(const QString &name);
        void disconnectFriends(const QString &name);
        void addFriend(const Plasma::DataEngine::Data &data);
        void addNearbyPerson(const Plasma::DataEngine::Data &data);

        // Configuration dialog
        Ui::opendesktopConfig ui;
        Ui::opendesktopLocationConfig locationUi;

        qlonglong id;
        Plasma::TabBar* m_tabs;

        QGraphicsLinearLayout* m_layout;

        // Personal tab
        UserWidget* m_userWidget;
        Plasma::DataEngine::Data m_ownData;

        // Friends tab
        Plasma::ScrollWidget* m_friendsScroll;
        QGraphicsWidget* m_friendsWidget;
        QHash<QString, ContactWidget*> m_friends;
        QGraphicsLinearLayout* m_friendsLayout;

        // Nearby people tab
        Plasma::ScrollWidget* m_nearScroll;
        Plasma::IconWidget* m_homeButton;
        QGraphicsWidget* m_nearWidget;
        QHash<QString, ContactWidget*> m_near;
        QGraphicsLinearLayout* m_nearLayout;

        // Config values
        int m_maximumItems;
        QString m_username;
        QString m_displayedUser;
        struct GeoLocation *m_geolocation;
        void saveGeoLocation();
        void syncGeoLocation();
};

#endif
