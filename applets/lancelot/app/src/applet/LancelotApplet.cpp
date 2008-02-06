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
        kDebug() << "DBUS connetcion to Lancelot failed!\n";
        setFailedToLaunch(true, i18n("Can't start Lancelot"));
    }

    QDBusReply<int> reply = m_lancelot->addClient();
    if (reply.isValid()) {
        m_clientID = reply.value();
    }
    
    
}

LancelotApplet::LancelotApplet(QObject *parent, const QVariantList &args) :
    Plasma::Applet(parent, args), m_lancelot(NULL),
    m_clientID(-1), m_instance(NULL), m_layout(NULL),
    m_showCategories(false), m_isVertical(false),
    m_configDialog(NULL)
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
    kDebug() << "Constraints are updated!\n";
    if (Plasma::FormFactorConstraint | constraints) {
        m_isVertical = (formFactor() == Plasma::Vertical);
        kDebug() << "I am going to be " << (m_isVertical?"VERTICAL":"NORMAL") << " from now on\n";
        layoutButtons();
    }
}

QSizeF LancelotApplet::contentSizeHint() const
{
    if (m_isVertical) {
        return QSizeF(48, 48 * (1 + m_buttons.size()));
    } else {
        return QSizeF(48 * (1 + m_buttons.size()), 48);
    }
}

void LancelotApplet::showLancelot()
{
    m_lancelot->show();
}

void LancelotApplet::showLancelotSection(const QString & section)
{
    m_lancelot->showItem(section);
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
    Plasma::Applet::updateGeometry();
    kDebug() << "updateGeometry()\n";
}


void LancelotApplet::deleteButtons()
{
    foreach (Lancelot::ExtenderButton * button, m_buttons) {
        m_layout->removeItem(button);
        button->setParent(NULL);
        delete button;
    }
    m_buttons.clear();
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
    if (m_buttons.size() == 0) {
        return;
    }
    
    qreal wpercent = 1.0 / (m_buttons.size());
    qreal distance = 0;

    foreach(Lancelot::ExtenderButton * button, m_buttons) {
        button->setGroupByName("AppletLaunchButton");
        button->setActivationMethod(m_clickActivation?(Lancelot::ExtenderButton::Click):(Lancelot::ExtenderButton::Hover));
        
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
}

void LancelotApplet::applyConfig()
{
    if (m_showCategories) {
        createCategories();
    } else {
        createMenuButton();
    }
    emit geometryChanged();
    layoutButtons();
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
    kcg.writeEntry("show", (m_clickActivation?"click":"hover"));
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
