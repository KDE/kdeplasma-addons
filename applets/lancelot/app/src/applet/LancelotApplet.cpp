/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotApplet.h"
#include "KDebug"
#include "KIcon"

#include <QDBusInterface>
#include <QDBusReply>

void LancelotApplet::dbusConnect() {
    m_lancelot = new org::kde::lancelot::App(
        "org.kde.lancelot",
        "/Lancelot",
        QDBusConnection::sessionBus()
    );

    if (!m_lancelot->isValid()) {
        kDebug() << "DBUS connection to Lancelot failed!\n";
        //setFailedToLaunch(true, i18n("Can't connect to Lancelot"));
    }

    QDBusReply<int> reply = m_lancelot->addClient();
    if (reply.isValid()) {
        m_clientID = reply.value();
    } else {
        setFailedToLaunch(true, i18n("Can't connect to Lancelot"));
    }
    
    
}

LancelotApplet::LancelotApplet(QObject *parent, const QVariantList &args) :
    Plasma::Applet(parent, args), m_lancelot(NULL),
    m_clientID(-1), m_instance(NULL), m_layout(NULL),
    m_configDialog(NULL), 
    m_isVertical(false), m_blockUpdates(false),
    m_showCategories(false), m_mainIcon(),
    m_clickActivation(false)
{
    setDrawStandardBackground(true);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);
    
    dbusConnect();
    
    // Instantiating Lancelot framework
    m_instance = new Lancelot::Instance();

    m_layout = new Plasma::NodeLayout();
    setLayout(m_layout);

    loadConfig();
    applyConfig();

    connect(
        & m_signalMapper, SIGNAL(mapped(const QString &)),
        this, SLOT(showLancelotSection(const QString &))
    );
        
    m_instance->activateAll();
}

LancelotApplet::~LancelotApplet()
{
    m_lancelot->removeClient(m_clientID);
    delete m_lancelot;
    
    deleteButtons();
    delete m_layout;
    
    delete m_instance;
}

Qt::Orientations LancelotApplet::expandingDirections() const
{
    return 0;
}

void LancelotApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & (Plasma::LocationConstraint | Plasma::SizeConstraint)) {
        //layoutButtons();
        updateGeometry();
    }
    if (constraints & Plasma::FormFactorConstraint) {
        m_isVertical = (formFactor() == Plasma::Vertical);
        kDebug() << "I am going to be " << (m_isVertical?"VERTICAL":"NORMAL") << " from now on\n";
        //layoutButtons();
        updateGeometry();
    }
}

QSizeF LancelotApplet::contentSizeHint() const
{
    if (m_buttons.size() <= 1) {
        return QSizeF(48, 48);
    }
    if (m_isVertical) {
        return QSizeF(48, 48 * (m_buttons.size()));
    } else {
        return QSizeF(48 * (m_buttons.size()), 48);
    }
}

void LancelotApplet::showLancelot()
{
    QPoint position = popupPosition(QSize());
    m_lancelot->show(position.x(), position.y());
}

void LancelotApplet::showLancelotSection(const QString & section)
{
    QPoint position = popupPosition(QSize());
    m_lancelot->showItem(position.x(), position.y(), section);
}

bool LancelotApplet::hasHeightForWidth () const
{
    return true;
}

qreal LancelotApplet::heightForWidth (qreal width) const
{
    if (m_buttons.size() == 0) {
        return width;
    }

    if (m_isVertical) {
        return width * m_buttons.size();
    } else {
        return width / (qreal)m_buttons.size();
    }
}

bool LancelotApplet::hasWidthForHeight () const
{
    return true;
}

qreal LancelotApplet::widthForHeight (qreal height) const
{
    if (m_buttons.size() == 0) {
        return height;
    }

    if (m_isVertical) {
        return height / (qreal)m_buttons.size();
    } else {
        return height * m_buttons.size();
    }
}

void LancelotApplet::updateGeometry ()
{
    if (m_blockUpdates) return;
    Plasma::Applet::updateGeometry();
    kDebug() << "updateGeometry()\n";
    layoutButtons();
}


void LancelotApplet::deleteButtons()
{
    m_blockUpdates = true;
    Lancelot::ExtenderButton * button;
    while (m_buttons.size() > 0) {
        button = m_buttons.takeLast();
        m_layout->removeItem(button);
        button->setParent(NULL);
        delete button;
    }
    m_buttons.clear();
    m_blockUpdates = false;
}

void LancelotApplet::createCategories()
{
    deleteButtons();

    // Loading Lancelot application categories
    QDBusReply<QStringList> replyIDs   = m_lancelot->sectionIDs();
    QDBusReply<QStringList> replyNames = m_lancelot->sectionNames();
    QDBusReply<QStringList> replyIcons = m_lancelot->sectionIcons();

    if (!replyIDs.isValid() || !replyNames.isValid() || !replyIcons.isValid()) {
        // Error connecting to Lancelot via d-bus
        setFailedToLaunch(true);
        return;
    } else {
        // Creating buttons...
        for (int i = 0; i < replyIDs.value().size(); i++) {
            Lancelot::ExtenderButton * button = new Lancelot::ExtenderButton(
                replyIDs.value().at(i),
                new KIcon(replyIcons.value().at(i)),
                "",
                "",
                this
            );

            connect(
                button, SIGNAL(activated()),
                & m_signalMapper, SLOT(map())
            );
            m_signalMapper.setMapping(button, replyIDs.value().at(i));

            m_buttons << button;
        }
    }
}

void LancelotApplet::createMenuButton()
{
    deleteButtons();

    Lancelot::ExtenderButton * button = new Lancelot::ExtenderButton(
        "launcher",
        new KIcon(m_mainIcon),
        "",
        "",
        this
    );

    //button->setGroupByName("AppletLaunchButton");
    connect(button, SIGNAL(activated()), this, SLOT(showLancelot()));

    m_buttons << button;
}

void LancelotApplet::layoutButtons()
{
    kDebug() << ".";
    if (m_buttons.size() == 0) {
        kDebug() << "Buttons size 0";
        return;
    }

    int iconSize = 48;
    
    kDebug() << "Geometry is " << contentSize();
    
    if (m_isVertical) {
        iconSize = (int)(qMin(contentSize().width(), contentSize().height() / m_buttons.size()));
    } else {
        iconSize = (int)(qMin(contentSize().width() / m_buttons.size(), contentSize().height()));
    }

    if (iconSize > 80) iconSize = 64;
    else if (iconSize > 60) iconSize = 48;
    else if (iconSize > 32) iconSize = 32;
    
    qreal wpercent = 1.0 / (m_buttons.size());
    qreal distance = 0;

    foreach(Lancelot::ExtenderButton * button, m_buttons) {
        button->setGroupByName("AppletLaunchButton");
        button->setActivationMethod(m_clickActivation?(Lancelot::ExtenderButton::Click):(Lancelot::ExtenderButton::Hover));
        button->setIconSize(QSize(iconSize, iconSize));
        
        if (m_isVertical) {
            m_layout->addItem(
                button,
                Plasma::NodeLayout::NodeCoordinate(0, distance),
                Plasma::NodeLayout::NodeCoordinate(1, distance + wpercent)
            );
        } else {
            m_layout->addItem(
                button,
                Plasma::NodeLayout::NodeCoordinate(distance, 0),
                Plasma::NodeLayout::NodeCoordinate(distance + wpercent, 1)
            );
        }
        distance += wpercent;
    }
    
    m_layout->invalidate();
}

void LancelotApplet::applyConfig()
{
    kDebug() << ".";
    if (m_showCategories) {
        createCategories();
    } else {
        createMenuButton();
    }
    emit geometryChanged();
    updateGeometry();
    //layoutButtons();
}
    
void LancelotApplet::loadConfig()
{
    KConfigGroup kcg =  config();
    m_showCategories =  (kcg.readEntry("show", "main") != "main");
    m_mainIcon =        kcg.readEntry("icon", "lancelot");
    m_clickActivation = (kcg.readEntry("activate", "click") == "click");
}

void LancelotApplet::saveConfig()
{
    KConfigGroup kcg = config();
    kcg.writeEntry("show", (m_showCategories?"categories":"main"));
    kcg.writeEntry("icon", m_mainIcon);
    kcg.writeEntry("activate", (m_clickActivation?"click":"hover"));
    save(&kcg);
}
  
void LancelotApplet::configAccepted()
{
    m_showCategories = m_configDialog->showCategories();
    m_mainIcon = m_configDialog->icon();
    m_clickActivation = m_configDialog->clickActivation();
    
    saveConfig();
    applyConfig();
}

void LancelotApplet::showConfigurationInterface()
{
    if (m_configDialog == NULL) {
        m_configDialog = new LancelotAppletConfig();
        connect(m_configDialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(m_configDialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    }

    m_configDialog->setShowCategories(m_showCategories);
    m_configDialog->setIcon(m_mainIcon);
    m_configDialog->setClickActivation(m_clickActivation);

    m_configDialog->show();
}

bool hasConfigurationInterface()      
{
    return true;
}


#include "LancelotApplet.moc"
