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

//own
#include "opendesktop.h"


//KDE
#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KCMultiDialog>

//plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/TabBar>
#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include <Plasma/ServiceJob>
#include <Plasma/ScrollWidget>
#include <Plasma/ToolTipManager>

#include "actionstack.h"
#include "contactlist.h"
#include "friendlist.h"
#include "loginwidget.h"
#include "messagecounter.h"
#include "messagelist.h"
#include "utils.h"


K_EXPORT_PLASMA_APPLET(opendesktop, OpenDesktop)

using namespace Plasma;

OpenDesktop::OpenDesktop(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
        m_tabs(0),
        m_loginWidget(0),
        m_friendList(0),
        m_friendStack(0),
        m_nearList(0),
        m_provider("https://api.opendesktop.org/v1/"),
        m_credentialsSource(QString("Credentials\\provider:%1").arg(m_provider)),
        m_kcmDialog(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_opendesktop");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setPassivePopup(true);

    setPopupIcon("system-users");
}

OpenDesktop::~OpenDesktop()
{
}

void OpenDesktop::init()
{
    (void)graphicsWidget();
    
    kDebug() << "init: opendesktop";

    m_engine->connectSource("Providers", this);

    configChanged();

    connectGeolocation();
}

void OpenDesktop::connectGeolocation()
{
    dataEngine("geolocation")->connectSource("location", this);
}

QGraphicsWidget* OpenDesktop::graphicsWidget()
{
    if (!m_tabs) {
        m_engine = dataEngine("ocs");

        // People near me
        m_nearList = new ContactList(m_engine);
        m_nearStack = new ActionStack(m_engine, m_nearList);
        
        m_nearStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_tabs = new Plasma::TabBar;

        m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        m_tabs->addTab(i18n("Nearby"), m_nearStack);

        connect(this, SIGNAL(providerChanged(QString)), m_nearList, SLOT(setProvider(QString)));
        connect(this, SIGNAL(providerChanged(QString)), m_nearStack, SLOT(setProvider(QString)));

        connect(m_nearList, SIGNAL(addFriend(QString)), m_nearStack, SLOT(addFriend(QString)));
        connect(m_nearList, SIGNAL(sendMessage(QString)), m_nearStack, SLOT(sendMessage(QString)));
        connect(m_nearList, SIGNAL(showDetails(QString)), m_nearStack, SLOT(showDetails(QString)));

        connect(m_nearStack, SIGNAL(endWork()), SLOT(endWork()));
        connect(m_nearStack, SIGNAL(startWork()), SLOT(startWork()));

        connect(this, SIGNAL(usernameChanged(QString)), m_nearList, SLOT(setOwnId(QString)));
        connect(this, SIGNAL(usernameChanged(QString)), m_nearStack, SLOT(setOwnId(QString)));
        
        emit providerChanged(m_provider);
    }
    return m_tabs;
}


void OpenDesktop::showLoginWidget(bool show)
{
    if (!show) {
        if (m_loginWidget) {
            m_tabs->removeTab(1);
            delete m_loginWidget;
            m_loginWidget = 0;
        }
        return;
    }
    
    if (!m_loginWidget) {
        m_loginWidget = new LoginWidget(m_engine);
        m_tabs->addTab(i18n("Login"), m_loginWidget);
        connect(m_loginWidget, SIGNAL(loginFinished()), this, SLOT(loginFinished()));
        connect(this, SIGNAL(providerChanged(QString)), m_loginWidget, SLOT(setProvider(QString)));

        m_loginWidget->setProvider(m_provider);
    }
}


void OpenDesktop::loginFinished()
{
    showLoginWidget(false);
    showFriendsWidget();
    emit providerChanged(m_provider);
    emit usernameChanged(m_user);
}


void OpenDesktop::showFriendsWidget()
{
    if (m_friendStack) {
        return;
    }

    // Messages
    m_messageCounter = new MessageCounter(m_engine, this);

    // Friends
    m_friendList = new FriendList(m_engine);
    m_friendStack = new ActionStack(m_engine, m_friendList);

    m_messageList = new MessageList(m_engine);
    m_messageList->setFolder("0");

    m_tabs->addTab(i18n("Friends"), m_friendStack);
    m_tabs->addTab(i18n("Messages"), m_messageList);

    connect(m_friendList, SIGNAL(addFriend(QString)), m_friendStack, SLOT(addFriend(QString)));
    connect(m_friendList, SIGNAL(sendMessage(QString)), m_friendStack, SLOT(sendMessage(QString)));
    connect(m_friendList, SIGNAL(showDetails(QString)), m_friendStack, SLOT(showDetails(QString)));

    connect(m_friendStack, SIGNAL(endWork()), SLOT(endWork()));
    connect(m_friendStack, SIGNAL(startWork()), SLOT(startWork()));

    connect(this, SIGNAL(usernameChanged(QString)), m_friendList, SLOT(setOwnId(QString)));
    connect(this, SIGNAL(usernameChanged(QString)), m_friendStack, SLOT(setOwnId(QString)));

    connect(this, SIGNAL(providerChanged(QString)), m_friendList, SLOT(setProvider(QString)));
    connect(this, SIGNAL(providerChanged(QString)), m_friendStack, SLOT(setProvider(QString)));
    connect(this, SIGNAL(providerChanged(QString)), m_messageList, SLOT(setProvider(QString)));
    connect(this, SIGNAL(providerChanged(QString)), m_messageCounter, SLOT(setProvider(QString)));

    m_friendList->setOwnId(m_user);
    m_friendStack->setOwnId(m_user);
    m_friendList->setProvider(m_provider);
    m_friendStack->setProvider(m_provider);

    m_messageList->setProvider(m_provider);
    m_messageCounter->setProvider(m_provider);
}


void OpenDesktop::connectNearby(qreal latitude, qreal longitude)
{
    QString src = QString("Near\\provider:%1\\latitude:%2\\longitude:%3\\distance:0")
        .arg(m_provider)
        .arg(latitude)
        .arg(longitude);

    m_nearList->setQuery(src);
}


void OpenDesktop::endWork()
{
    // FIXME: Count
    setBusy(false);
}


void OpenDesktop::startWork()
{
    // FIXME: Count
    setBusy(true);
}


void OpenDesktop::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    kDebug() << "source updated:" << source << data;

    m_tabs->setPreferredSize(-1, -1);
    emit sizeHintChanged(Qt::PreferredSize);

    if (source == "location") {
        // The location from the geolocation engine arrived!
        m_geolocation.city = data["city"].toString();
        m_geolocation.country = data["country"].toString();
        m_geolocation.countryCode = data["country code"].toString();
        m_geolocation.accuracy = data["accuracy"].toInt();
        m_geolocation.latitude = data["latitude"].toDouble();
        m_geolocation.longitude = data["longitude"].toDouble();
        kDebug() << "geolocation:" << m_geolocation.city << m_geolocation.country <<
                m_geolocation.countryCode << m_geolocation.latitude << m_geolocation.longitude;
        connectNearby(m_geolocation.latitude, m_geolocation.longitude);
        saveGeoLocation();
    } else if (source == m_credentialsSource) {
        m_user = data["UserName"].toString();
        m_password = data["Password"].toString();
        ui.username->setText(m_user);
        ui.password->setText(m_password);
    } else if (source == "Providers") {
        // The provider to use has been loaded... and it tells about a user name to use
        if (data.contains(m_provider)) {
            QHash<QString, QVariant> hashData = data[m_provider].toHash();
            QString user = hashData.value("UserName").toString();
            kDebug() << "USER" << user;

            m_user = user;
            if (user.isEmpty()) {
                showLoginWidget(true);
            } else {
                showFriendsWidget();
                emit usernameChanged(user);
            }
        }
    }
}


void OpenDesktop::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *generalSettingswidget = new QWidget(parent);
    ui.setupUi(generalSettingswidget);
    parent->addPage(generalSettingswidget, i18n("General"), Applet::icon());

    m_engine->connectSource(m_credentialsSource, this);
    
    QVariant providers = m_engine->query("Providers");
    kDebug() << providers;
    QVariantHash p = providers.toHash();
    
    foreach(const QString& key, p.keys()) {
        QString name = p.value(key).toHash().value("Name").toString();
        QString id = key;
        ui.provider->addItem(name, id);
    }
    
    // FIXME connect current changed to get different user name

    QWidget *locationWidget = new QWidget(parent);
    locationUi.setupUi(locationWidget);
    parent->addPage(locationWidget, i18n("Location"), "go-home");
    // TODO: connect finished() signal to null the ui

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.registerButton, SIGNAL(clicked()), this, SLOT(registerAccount()));
    connect(locationUi.publishLocation, SIGNAL(clicked()), this, SLOT(publishGeoLocation()));

    locationUi.city->setText(m_geolocation.city);
    locationUi.latitude->setText(QString::number(m_geolocation.latitude));
    locationUi.longitude->setText(QString::number(m_geolocation.longitude));

    locationUi.countryCombo->setInsertPolicy(QComboBox::InsertAlphabetically);
    foreach ( const QString &cc, KGlobal::locale()->allCountriesList() ) {
        locationUi.countryCombo->addItem(KGlobal::locale()->countryCodeToName(cc), cc);
    }
    locationUi.countryCombo->setCurrentIndex(locationUi.countryCombo->findText(KGlobal::locale()->countryCodeToName(m_geolocation.countryCode)));

    // actually, 0,0 is a valid location, but here we're using it to see if we
    // actually have a location, a bit dirty but far less complex, especially given
    // that this point is located in the middle of the ocean off the coast of Ghana
    if (m_geolocation.latitude == 0 && m_geolocation.longitude == 0) {
        locationUi.publishLocation->setEnabled(false);
        
    connect(ui.provider , SIGNAL(currentIndexChanged(int)), parent , SLOT(settingsModified()));
    connect(ui.username , SIGNAL(textChanged(QString)), parent ,SLOT(settingsModified()));
    connect(ui.password, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.registerButton, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(locationUi.city , SIGNAL(textChanged(QString)), parent , SLOT(settingsModified()));
    connect(locationUi.countryCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(locationUi.latitude, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(locationUi.longitude, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(locationUi.publishLocation, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    }
}

void OpenDesktop::configAccepted()
{
    QString provider = ui.provider->itemData(ui.provider->currentIndex()).toString();
    if(provider != m_provider) {
        kDebug() << "Provider changed" << provider;
        KConfigGroup cg = config();
        cg.writeEntry("provider", m_provider);
        emit configNeedsSaving();
    }
    if (!ui.username->text().isEmpty()) {
        Service* service = m_engine->serviceForSource(settingsQuery(m_provider, "setCredentials"));
        KConfigGroup cg = service->operationDescription("setCredentials");
        kDebug() << ui.username->text() << "in config group...";
        cg.writeEntry("username", ui.username->text());
        cg.writeEntry("password", ui.password->text());
        ServiceJob* job = service->startOperationCall(cg);
        connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
    }
    syncGeoLocation();
}

void OpenDesktop::configChanged()
{
    KConfigGroup cg = config();
    m_geolocation.city = cg.readEntry("geoCity", QString());
    m_geolocation.country = cg.readEntry("geoCountry", QString());
    m_geolocation.countryCode = cg.readEntry("geoCountryCode", QString());
    m_geolocation.latitude = cg.readEntry("geoLatitude", 0.0);
    m_geolocation.longitude = cg.readEntry("geoLongitude", 0.0);
    
    QString provider = cg.readEntry("provider", QString("https://api.opendesktop.org/v1/"));
    if (provider != m_provider) {
        m_provider = provider;
        m_credentialsSource = QString("Credentials\\provider:%1").arg(m_provider);
        emit providerChanged(m_provider);
    }
}

void OpenDesktop::registerAccount()
{
    kDebug() << "register new account";
    if (m_kcmDialog) {
        m_kcmDialog->show();
        return;
    }
    m_kcmDialog = new KCMultiDialog();
    connect(m_kcmDialog, SIGNAL(finished()), this, SLOT(kcm_finished()));
    m_kcmDialog->addModule("kcm_attica");
    m_kcmDialog->setWindowTitle(i18nc("title of control center dialog to configure providers for community applet", "Provider Configuration - Community Plasma Applet"));
    m_kcmDialog->show();   
}

void OpenDesktop::kcm_finished()
{
    m_kcmDialog->deleteLater();
    m_kcmDialog = 0;
}

void OpenDesktop::syncGeoLocation()
{
    // Location tab
    m_geolocation.city = locationUi.city->text();
    m_geolocation.countryCode = locationUi.countryCombo->itemData(locationUi.countryCombo->currentIndex()).toString();
    m_geolocation.country = locationUi.countryCombo->currentText();
    m_geolocation.latitude = locationUi.latitude->text().toDouble();
    m_geolocation.longitude = locationUi.longitude->text().toDouble();

    kDebug() << "New location:" << m_geolocation.city << m_geolocation.country << m_geolocation.countryCode << m_geolocation.latitude << m_geolocation.longitude;

    saveGeoLocation();
}

void OpenDesktop::publishGeoLocation()
{
    syncGeoLocation();
    // FIXME: Use service
    QString source = QString("PostLocation-%1:%2:%3:%4").arg(
                                    QString("%1").arg(m_geolocation.latitude),
                                    QString("%1").arg(m_geolocation.longitude),
                                    m_geolocation.countryCode,
                                    m_geolocation.city);
    kDebug() << "updating location:" << source;
    m_engine->connectSource(source, this);
}

void OpenDesktop::saveGeoLocation()
{
    KConfigGroup cg = config();
    cg.writeEntry("geoCity", m_geolocation.city);
    cg.writeEntry("geoCountry", m_geolocation.country);
    cg.writeEntry("geoCountryCode", m_geolocation.countryCode);
    cg.writeEntry("geoLatitude", m_geolocation.latitude);
    cg.writeEntry("geoLongitude", m_geolocation.longitude);

    emit configNeedsSaving();
}


void OpenDesktop::unreadMessageCountChanged(int count)
{
    if (count) {
        m_tabs->setTabText(2, i18n("Messages (%1)", count));
    } else {
        m_tabs->setTabText(2, i18n("Messages"));
    }
}


#include "opendesktop.moc"
