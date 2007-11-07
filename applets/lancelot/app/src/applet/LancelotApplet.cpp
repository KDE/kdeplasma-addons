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

LancelotApplet::LancelotApplet(QObject *parent, const QVariantList &args) :
    Plasma::Applet(parent, args)
{
    setDrawStandardBackground(true);
    m_instance = new Lancelot::Instance();

    // Connecting to Lancelot application via DBus
    m_dbus = new QDBusInterface( "org.kde.lancelot", "/Lancelot", "org.kde.lancelot.App");
    QDBusReply<int> reply = m_dbus->call("addClient");
    if (reply.isValid()) {
        m_clientID = reply.value();
    }

    QDBusReply<QStringList> replyIDs   = m_dbus->call("sectionIDs");
    QDBusReply<QStringList> replyNames = m_dbus->call("sectionNames");
    QDBusReply<QStringList> replyIcons = m_dbus->call("sectionIcons");

    // creating buttons...
    m_layout = new Plasma::NodeLayout();
    
    kDebug();
    if (replyIDs.isValid() && replyNames.isValid() && replyIcons.isValid()) {
        for (int i = 0; i < replyIDs.value().size(); i++) {
            kDebug() << replyIDs.value().at(i) << replyIcons.value().at(i) << replyNames.value().at(i);
            m_sectionButtons << new Lancelot::ExtenderButton(
                replyIDs.value().at(i),
                new KIcon(replyIcons.value().at(i)),
                "", "", this
            );
        }
    }
    kDebug();
    
    qreal wpercent = 1.0 / (m_sectionButtons.size() + 1.0);
    
    m_layout->addItem(
        m_buttonMain = new Lancelot::ExtenderButton("launcher", new KIcon("lancelot"), "", "", this),
        Plasma::NodeLayout::NodeCoordinate(0, 0),
        Plasma::NodeLayout::NodeCoordinate(wpercent, 1)
    );
    m_buttonMain->setGroupByName("AppletLaunchButton");
    
    qreal left = wpercent;
    
    foreach(Lancelot::ExtenderButton * button, m_sectionButtons) {
        button->setGroupByName("AppletLaunchButton");
        m_layout->addItem(
            button,
            Plasma::NodeLayout::NodeCoordinate(left, 0),
            Plasma::NodeLayout::NodeCoordinate(left + wpercent, 1)
        );
        left += wpercent;
    }

    m_buttonMain->setActivationMethod(Lancelot::ExtenderButton::Hover);

    m_instance->activateAll();
    m_layout->setGeometry(QRectF(QPoint(), contentSize()));
    
    setAcceptsHoverEvents(true);

    connect(m_buttonMain, SIGNAL(activated()), this, SLOT(showLancelot()));


}

LancelotApplet::~LancelotApplet()
{
    m_dbus->call( "removeClient", "id" );
    delete m_dbus;
    delete m_layout;
    delete m_buttonMain;
    delete m_instance;
}

void LancelotApplet::setGeometry(const QRectF & geometry)
{
    Applet::setGeometry(geometry);
    m_layout->setGeometry(QRectF(QPoint(), contentSize()));
}

QSizeF LancelotApplet::contentSizeHint() const
{
    return QSizeF(48 * (1 + m_sectionButtons.size()), 48);
}

void LancelotApplet::showLancelot()
{
    m_dbus->call("show");
}

void LancelotApplet::showLancelotSection(const QString & section)
{
    m_dbus->call("showItem");
}

qreal LancelotApplet::heightForWidth(qreal width) const
{
    return width / (1 + m_sectionButtons.size());
}


#include "LancelotApplet.moc"
