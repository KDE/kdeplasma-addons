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

//own
#include "opendesktop.h"
#include "activitylist.h"
#include "contactlist.h"
#include "utils.h"

//Qt

//KDE
#include <KConfigDialog>
#include <KLocale>
#include <KToolInvocation>

//plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/TabBar>
#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include <Plasma/ScrollWidget>
#include <Plasma/ToolTipManager>

K_EXPORT_PLASMA_APPLET(opendesktop, OpenDesktop)

using namespace Plasma;

struct GeoLocation {
    QString country;
    QString city;
    QString countryCode;
    int accuracy;
    qreal latitude;
    qreal longitude;
    qreal distance;
};


OpenDesktop::OpenDesktop(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
        id(0),
        m_tabs(0),
        m_friendList(0),
        m_nearList(0),
        m_maximumItems(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_opendesktop");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setPassivePopup(true);

    setPopupIcon("system-users");

    if (args.count() > 0) {
        id = args.at(0).toString().toInt();
        kDebug() << "Arguments:" << args << "id:" << args.at(0).toString().toInt();
    }

    m_geolocation = new GeoLocation;


    (void)graphicsWidget();
}

OpenDesktop::~OpenDesktop()
{
    delete m_geolocation;
}

void OpenDesktop::init()
{
    kDebug() << "init: opendesktop";
    KConfigGroup cg = config();
    m_geolocation->city = cg.readEntry("geoCity", QString());
    m_geolocation->country = cg.readEntry("geoCountry", QString());
    m_geolocation->countryCode = cg.readEntry("geoCountryCode", QString());
    m_geolocation->latitude = cg.readEntry("geoLatitude", 0);
    m_geolocation->longitude = cg.readEntry("geoLongitude", 0);
    m_geolocation->distance = cg.readEntry("geoDistance", 5);
    m_maximumItems = cg.readEntry("maximumItems", 64);
    dataEngine("ocs")->connectSource("MaximumItems-" + QString::number(m_maximumItems), this);
    if (!id) {
        id = cg.readEntry("currentId", 0);
    } else {
        cg.writeEntry("currentId", id);
        emit configNeedsSaving();
    }
    m_username = cg.readEntry("username", QString());
    m_displayedUser = m_username;
    resize(420, 420);

    if (m_username.isEmpty()) {
        setConfigurationRequired(true);
    } else {
        connectGeolocation();
    }
    switchDisplayedUser(m_username, false);
    m_friendList->setOwnId(m_username);
    m_nearList->setOwnId(m_username);
}

void OpenDesktop::connectGeolocation()
{
    dataEngine("geolocation")->connectSource("location", this);
}


QGraphicsWidget* OpenDesktop::graphicsWidget()
{
    if (!m_tabs) {
        m_tabs = new Plasma::TabBar(this);
        m_tabs->setPreferredSize(300, 400);
        m_tabs->setMinimumSize(150, 200);
        m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Friends activity
        m_activityList = new ActivityList(dataEngine("ocs"), m_tabs);
        m_tabs->addTab(i18n("News feed"), m_activityList);

        m_userWidget = new UserWidget(dataEngine("ocs"), m_tabs);
        m_tabs->addTab(i18n("Personal"), m_userWidget);

        // Friends
        m_friendList = new ContactList(dataEngine("ocs"), m_tabs);
        m_tabs->addTab(i18n("Friends"), m_friendList);
        connect(m_friendList, SIGNAL(addFriend(QString)), SLOT(addFriend(QString)));
        connect(m_friendList, SIGNAL(sendMessage(QString)), SLOT(sendMessage(QString)));
        connect(m_friendList, SIGNAL(showDetails(QString)), SLOT(switchDisplayedUser(QString)));

        // People near me
        m_nearList = new ContactList(dataEngine("ocs"), m_tabs);
        m_tabs->addTab(i18n("Nearby"), m_nearList);
        connect(m_nearList, SIGNAL(addFriend(QString)), SLOT(addFriend(QString)));
        connect(m_nearList, SIGNAL(sendMessage(QString)), SLOT(sendMessage(QString)));
        connect(m_nearList, SIGNAL(showDetails(QString)), SLOT(switchDisplayedUser(QString)));

        // "Home" button, outside of the layout
        m_homeButton = new Plasma::IconWidget(this);
        m_homeButton->setIcon("go-home");
        m_homeButton->setGeometry(QRectF(contentsRect().topLeft(),
                                  QSizeF(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium)));
        connect(m_homeButton, SIGNAL(clicked()), this, SLOT(goHome()));
        m_homeButton->setVisible(false);
    }
    return m_tabs;
}


void OpenDesktop::switchDisplayedUser(const QString& id, bool switchToPersonal)
{
    m_displayedUser = id;

    if (switchToPersonal) {
        m_tabs->setCurrentIndex(1);
    }
    m_userWidget->setId(id);
    if (!m_displayedUser.isEmpty()) {
        m_friendList->setQuery(friendsQuery(id));
    } else {
        m_friendList->setQuery(QString());
    }
    m_nearList->setQuery(QString());

    m_homeButton->setVisible(m_username != m_displayedUser);
}


void OpenDesktop::addFriend(const QString& id)
{
    KToolInvocation::invokeBrowser(QString("https://www.opendesktop.org/usermanager/relationadd.php?username=%1").arg(id));
}


void OpenDesktop::sendMessage(const QString& id)
{
     KToolInvocation::invokeBrowser(QString("https://www.opendesktop.org/messages/?action=newmessage&username=%1").arg(id));
}


void OpenDesktop::goHome()
{
    switchDisplayedUser(m_username);
}

void OpenDesktop::connectNearby(int latitude, int longitude)
{
    QString src = QString("Near-%1:%2:%3").arg(
                        QString::number(latitude),
                        QString::number(longitude),
                        QString::number(m_geolocation->distance));
    kDebug() << "geolocation src" << src;

    m_nearList->setQuery(src);
    kDebug() << "connected near";
}

void OpenDesktop::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << "source updated:" << source << data;
    if (source == "location") {
        // The location from the geolocation engine arrived!
        m_geolocation->city = data["city"].toString();
        m_geolocation->country = data["country"].toString();
        m_geolocation->countryCode = data["country code"].toString();
        m_geolocation->accuracy = data["accuracy"].toInt();
        m_geolocation->latitude = data["latitude"].toDouble();
        m_geolocation->longitude = data["longitude"].toDouble();
        kDebug() << "geolocation:" << m_geolocation->city << m_geolocation->country <<
                m_geolocation->countryCode << m_geolocation->latitude << m_geolocation->longitude;
        connectNearby(m_geolocation->latitude, m_geolocation->longitude);
        saveGeoLocation();
        return;
    }

    kDebug() << "Don't know what to do with" << source;
}


void OpenDesktop::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), Applet::icon());

    QWidget *locationWidget = new QWidget(parent);
    locationUi.setupUi(locationWidget);
    parent->addPage(locationWidget, i18n("Location"), "go-home");
    // TODO: connect finished() signal to null the ui

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.registerButton, SIGNAL(clicked()), this, SLOT(registerAccount()));
    connect(locationUi.publishLocation, SIGNAL(clicked()), this, SLOT(publishGeoLocation()));
    ui.maxitems->setValue(m_maximumItems);
    ui.username->setText(m_username);

    locationUi.city->setText(m_geolocation->city);
    locationUi.latitude->setText(QString::number(m_geolocation->latitude));
    locationUi.longitude->setText(QString::number(m_geolocation->longitude));
    locationUi.distance->setValue(m_geolocation->distance);

    locationUi.countryCombo->setInsertPolicy(QComboBox::InsertAlphabetically);
    foreach ( const QString &cc, KGlobal::locale()->allCountriesList() ) {
        locationUi.countryCombo->addItem(KGlobal::locale()->countryCodeToName(cc), cc);
    }
    locationUi.countryCombo->setCurrentIndex(locationUi.countryCombo->findText(KGlobal::locale()->countryCodeToName(m_geolocation->countryCode)));

    // actually, 0,0 is a valid location, but here we're using it to see if we
    // actually have a location, a bit dirty but far less complex, especially given
    // that this point is located in the middle of the ocean off the coast of Ghana
    if (m_geolocation->latitude == 0 && m_geolocation->longitude == 0) {
        locationUi.publishLocation->setEnabled(false);
    }
}

void OpenDesktop::configAccepted()
{
    KConfigGroup cg = config();

    // General tab
    QString cuser = ui.username->text();
    if (m_username != cuser) {
        m_username = cuser;
        switchDisplayedUser(m_username);
        m_friendList->setOwnId(m_username);
        m_nearList->setOwnId(m_username);
        if (!m_username.isEmpty()) {
            connectGeolocation();
        }
        cg.writeEntry("username", m_username);
        emit configNeedsSaving();
        setConfigurationRequired(false);
    }
    int cmax = ui.maxitems->value();
    if (m_maximumItems != cmax) {
        m_maximumItems = cmax;
        cg.writeEntry("maximumItems", m_maximumItems);
        emit configNeedsSaving();
        dataEngine("ocs")->connectSource("MaximumItems-" + QString::number(m_maximumItems), this);
    }

    syncGeoLocation();
}

void OpenDesktop::registerAccount()
{
    kDebug() << "register new account";
    KToolInvocation::invokeBrowser("https://www.opendesktop.org/usermanager/new.php");
}

void OpenDesktop::syncGeoLocation()
{
    // Location tab
    m_geolocation->city = locationUi.city->text();
    m_geolocation->countryCode = locationUi.countryCombo->itemData(locationUi.countryCombo->currentIndex()).toString();
    m_geolocation->country = locationUi.countryCombo->currentText();
    m_geolocation->latitude = locationUi.latitude->text().toDouble();
    m_geolocation->longitude = locationUi.longitude->text().toDouble();
    m_geolocation->distance = locationUi.distance->text().toDouble();

    kDebug() << "New location:" << m_geolocation->city << m_geolocation->country << m_geolocation->countryCode << m_geolocation->latitude << m_geolocation->longitude;

    saveGeoLocation();
}

void OpenDesktop::publishGeoLocation()
{
    syncGeoLocation();
    QString source = QString("PostLocation-%1:%2:%3:%4").arg(
                                    QString("%1").arg(m_geolocation->latitude),
                                    QString("%1").arg(m_geolocation->longitude),
                                    m_geolocation->countryCode,
                                    m_geolocation->city);
    kDebug() << "updating location:" << source;
    dataEngine("ocs")->connectSource(source, this);
}

void OpenDesktop::saveGeoLocation()
{
    KConfigGroup cg = config();
    cg.writeEntry("geoCity", m_geolocation->city);
    cg.writeEntry("geoCountry", m_geolocation->country);
    cg.writeEntry("geoCountryCode", m_geolocation->countryCode);
    cg.writeEntry("geoLatitude", m_geolocation->latitude);
    cg.writeEntry("geoLongitude", m_geolocation->longitude);
    cg.writeEntry("geoDistance", m_geolocation->distance);

    emit configNeedsSaving();
}

#include "opendesktop.moc"
