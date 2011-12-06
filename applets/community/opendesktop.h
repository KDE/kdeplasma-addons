/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *   Copyright 2009 by Frederik Gladhorn <gladhorn@kde.org>                *
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

//Plasma
#include <Plasma/DataEngine>
#include <Plasma/PopupApplet>

#include "ui_opendesktopConfig.h"
#include "ui_opendesktopLocationConfig.h"


namespace Plasma
{
    class DataEngine;
    class TabBar;
}
class KCMultiDialog;

class ActionStack;
class ContactList;
class FriendList;
class LoginWidget;
class KConfigDialog;
class MessageCounter;
class MessageList;
class QGraphicsLinearLayout;

struct GeoLocation {
    QString country;
    QString city;
    QString countryCode;
    int accuracy;
    qreal latitude;
    qreal longitude;
};

class OpenDesktop : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        OpenDesktop(QObject *parent, const QVariantList &args);
        ~OpenDesktop();
        void init();
        QGraphicsWidget* graphicsWidget();

    Q_SIGNALS:
        void providerChanged(const QString& provider);
        void usernameChanged(const QString& user);

    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
        void endWork();
        void configAccepted();
        void configChanged();
        void startWork();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private Q_SLOTS:
        void publishGeoLocation();
        void registerAccount();
        void unreadMessageCountChanged(int count);
        
        void showLoginWidget(bool show);
        void showFriendsWidget();
        
        void loginFinished();
        void kcm_finished();
        
    private:
        void connectGeolocation();
        void connectNearby(qreal latitude, qreal longitude);

        // Configuration dialog
        Ui::opendesktopConfig ui;
        Ui::opendesktopLocationConfig locationUi;

        Plasma::TabBar* m_tabs;

        QGraphicsLinearLayout* m_layout;

        // Login tab
        LoginWidget* m_loginWidget;
        
        // Friends tab
        FriendList* m_friendList;
        ActionStack* m_friendStack;

        // Nearby people tab
        ContactList* m_nearList;
        ActionStack* m_nearStack;

        // Messages tab
        MessageList* m_messageList;

        // Config values
        QString m_provider;
        GeoLocation m_geolocation;
        void saveGeoLocation();
        void syncGeoLocation();
        MessageCounter* m_messageCounter;
        
        Plasma::DataEngine* m_engine;
        QString m_user;
        QString m_password;
        QString m_credentialsSource;
        
        // show attica config
        KCMultiDialog* m_kcmDialog;
};

#endif
