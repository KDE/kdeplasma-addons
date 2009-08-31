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
#include "contactwidget.h"

//Qt

//KDE
#include <KConfigDialog>
#include <KLocale>
#include <KRun>

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
        m_friendsScroll(0),
        m_friendsWidget(0),
        m_friendsLayout(0),
        m_nearScroll(0),
        m_nearWidget(0),
        m_nearLayout(0),
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

    if (m_username.isEmpty()) {
        setConfigurationRequired(true);
    } else {
        connectPerson(m_username);
        connectFriends(m_username);
        connectGeolocation();
    }
}

void OpenDesktop::connectGeolocation()
{
    dataEngine("geolocation")->connectSource("location", this);
}

void OpenDesktop::connectFriends(const QString &name)
{
    QString src = QString("Friends-%1").arg(name);
    dataEngine("ocs")->connectSource(src, this);
    //kDebug() << "connected friends";
}

void OpenDesktop::connectPerson(const QString &name)
{
    QString src = QString("Person-%1").arg(name);
    dataEngine("ocs")->connectSource(src, this);
    //kDebug() << "connected user person" << src;
}

void OpenDesktop::disconnectPerson(const QString &name)
{
    Q_UNUSED( name )
    QString src = QString("Person-%1").arg(m_username);
    dataEngine("ocs")->disconnectSource(src, this);

    //kDebug() << "disconnected user person" << name;
}

void OpenDesktop::disconnectFriends(const QString &name)
{
    QString src = QString("Friends-%1").arg(name);
    dataEngine("ocs")->disconnectSource(src, this);
    //kDebug() << "disconnected friends" << src;
}


QGraphicsWidget* OpenDesktop::graphicsWidget()
{
    if (!m_tabs) {
        m_tabs = new Plasma::TabBar(this);
        m_tabs->setPreferredSize(300, 400);
        m_tabs->setMinimumSize(150, 200);
        m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_userWidget = new UserWidget(m_tabs);
        m_tabs->addTab(i18n("Personal"), m_userWidget);

        // Friends
        m_friendsScroll = new Plasma::ScrollWidget(m_tabs);
        m_friendsWidget = new QGraphicsWidget(m_friendsScroll);
        m_friendsLayout = new QGraphicsLinearLayout(Qt::Vertical, m_friendsWidget);
        m_friendsScroll->setWidget(m_friendsWidget);
        m_tabs->addTab(i18n("Friends"), m_friendsScroll);

        // People near me
        m_nearScroll = new Plasma::ScrollWidget(m_tabs);
        m_nearWidget = new QGraphicsWidget(m_nearScroll);
        m_nearLayout = new QGraphicsLinearLayout(Qt::Vertical, m_nearWidget);
        m_nearScroll->setWidget(m_nearWidget);
        m_tabs->addTab(i18n("Nearby"), m_nearScroll);

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

void OpenDesktop::addFriend(const Plasma::DataEngine::Data &data)
{
    QString name = data["Name"].toString();
    QString _id = data["Id"].toString();
    kDebug() << "name, id" << name << _id;
    if (_id == m_username) {
        kDebug() << "Updating myself" << m_username;
        m_userWidget->setAtticaData(data);
        return;
    }

    if (m_friends.value(_id)) {
        kDebug() << "Updated existing widget" << (QObject*)m_near[_id];
        m_friends[_id]->setAtticaData(data);
        return;
    }

    if (name.isEmpty()) {
        kDebug() << "Name empty, using id" << name << _id;
        name = _id;
    }

    // just show the first couple of widgets
    if ( m_friends.count() > m_maximumItems ) {
        return;
    }

    ContactWidget* contactWidget = new ContactWidget(m_friendsWidget);
    contactWidget->setAtticaData(data);
    contactWidget->setIsFriend(true);
    connect(contactWidget, SIGNAL(showDetails(Plasma::DataEngine::Data)),
            this, SLOT(showDetails(Plasma::DataEngine::Data)));
    m_friendsLayout->addItem(contactWidget);
    m_friends[_id] = contactWidget;
}

void OpenDesktop::addNearbyPerson(const Plasma::DataEngine::Data &data)
{
    QString name = data["Name"].toString();
    QString _id = data["Id"].toString();
    kDebug() << "nearby name, id" << name << _id;

    if (_id == m_username) {
        kDebug() << "Updating myself" << m_username;
        m_userWidget->setAtticaData(data);
        return;
    }

    if (m_near.value(_id)) {
        kDebug() << "Updated existing widget" << (QObject*)m_near[_id];
        m_near[_id]->setAtticaData(data);
        return;
    }

    if (name.isEmpty()) {
        name = _id;
    }

    // just show the first couple of widgets
    if (m_near.count() > m_maximumItems) {
        return;
    }

    ContactWidget* contactWidget = new ContactWidget(m_friendsWidget);
    contactWidget->setAtticaData(data);
    connect(contactWidget, SIGNAL(showDetails(Plasma::DataEngine::Data)),
            this, SLOT(showDetails(Plasma::DataEngine::Data)));
    m_nearLayout->addItem(contactWidget);
    m_near[_id] = contactWidget;
}

void OpenDesktop::showDetails(const Plasma::DataEngine::Data &data)
{
    //kDebug() << "showing details. ... switching to user info tab";
    m_tabs->setCurrentIndex(0);
    m_userWidget->setAtticaData(data);

    int n = m_friendsLayout->count();
    for (int i = 0; i < n; i++) {
        ContactWidget *cw = dynamic_cast<ContactWidget*>(m_friendsLayout->itemAt(i));
        m_friends.remove(cw->user());
        cw->deleteLater();
    }

    n = m_nearLayout->count();
    for (int i = 0; i < n; i++) {
        ContactWidget *cw = dynamic_cast<ContactWidget*>(m_nearLayout->itemAt(i));
        m_near.remove(cw->user());
        cw->deleteLater();
    }

    disconnectPerson(m_displayedUser);
    disconnectFriends(m_displayedUser);
    m_displayedUser = data["Id"].toString();
    connectPerson(m_displayedUser);
    connectFriends(m_displayedUser);
    connectNearby(data["Latitude"].toDouble(), data["Longitude"].toDouble());

    m_homeButton->setVisible(m_username != m_displayedUser);
}

void OpenDesktop::goHome()
{
    showDetails(m_ownData);
}

void OpenDesktop::connectNearby(int latitude, int longitude)
{
    QString src = QString("Near-%1:%2:%3").arg(
                        QString::number(latitude),
                        QString::number(longitude),
                        QString::number(m_geolocation->distance));
    kDebug() << "geolocation src" << src;

    dataEngine("ocs")->connectSource(src, this);
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

    } else if (source.startsWith("Person-")) {
        // ourselves?
        Plasma::DataEngine::Data personData = data[source].value<Plasma::DataEngine::Data>();
        if (!personData["Id"].toString().isEmpty()) {
            QString self = QString("Person-%1").arg(m_username);
            if (data["Id"].toString() == m_username) {
                // Our own data has updated ...
                m_userWidget->setAtticaData(data);
                m_ownData = personData;
            }
            addFriend(personData);
        }
        return;

    } else if (source.startsWith("Near-")) {
        foreach (const QString &person, data.keys()) {
            if (person.startsWith("Person-")) {
                Plasma::DataEngine::Data personData = data[person].value<Plasma::DataEngine::Data>();
                addNearbyPerson(personData);
            }
        }
        return;
    } else if (source.startsWith("Friends-")) {
        foreach (const QString &person, data.keys()) {
            if (person.startsWith("Person-")) {
                Plasma::DataEngine::Data personData = data[person].value<Plasma::DataEngine::Data>();
                addFriend(personData);
            }
        }
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
        if (m_friends.value(cuser)) {
            m_userWidget->setAtticaData(m_friends[cuser]->data());
        }

        disconnectFriends(m_username);
        int n = m_friendsLayout->count();
        // Empty the friendslayout
        for (int i = 0; i < n; i++) {
            ContactWidget *cw = dynamic_cast<ContactWidget*>(m_friendsLayout->itemAt(i));
            m_friends.remove(cw->user());
            cw->deleteLater();
        }

        disconnectPerson(m_username);
        m_username = cuser;
        m_displayedUser = m_username;
        connectPerson(m_username);
        connectFriends(m_username);
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
    KUrl url("https://www.opendesktop.org/usermanager/new.php");
    kDebug() << "register new account" << url.url();
    new KRun(url, ui.registerButton);
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
