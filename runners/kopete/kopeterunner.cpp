/*
 * Copyright 2009 Ben Boeckel <MathStuf@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Header include
#include "kopeterunner.h"

// Qt includes
#include <QtCore/QHashIterator>
#include <QtCore/QUuid>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

// KDE includes
#include <KAboutData>
#include <KIcon>
#include <KUrl>

using namespace Plasma;

// DBus information for Kopete
static const QString KopeteDBusService   = "org.kde.kopete";
static const QString KopeteDBusPath      = "/Kopete";
static const QString KopeteDBusInterface = "org.kde.Kopete";

static QDBusMessage generateMethodCall(const QString& method)
{
    return QDBusMessage::createMethodCall(KopeteDBusService, KopeteDBusPath, KopeteDBusInterface, method);
}

KopeteRunner::KopeteRunner(QObject* parent, const QVariantList& args) :
        AbstractRunner(parent, args),
        m_loaded(false),
        m_checkLoaded(false)
{
    Q_UNUSED(args);
    setObjectName("Kopete contacts");

    setIgnoredTypes(RunnerContext::Directory | RunnerContext::File | RunnerContext::NetworkLocation);
    addSyntax(RunnerSyntax(":q:", i18n("Searches your Kopete buddylist for contacts matching :q:.")));
    addSyntax(RunnerSyntax("connect", i18n("Connect all Kopete accounts")));
    addSyntax(RunnerSyntax("disconnect", i18n("Disconnect all Kopete accounts")));
    addSyntax(RunnerSyntax("status :q:", i18n("Set Kopete accounts to a status with an optional message")));
    addSyntax(RunnerSyntax("message :q:", i18n("Set Kopete status message")));

    // Connect up signals
    connect(this, SIGNAL(prepare()), SLOT(slotPrepare()));
    connect(this, SIGNAL(teardown()), SLOT(slotTeardown()));
}

KopeteRunner::~KopeteRunner()
{
}

void KopeteRunner::match(Plasma::RunnerContext& context)
{
    if (!m_loaded) {
        return;
    }

    const QString term = context.query().toLower();
    if (term.length() < 3) {
        return;
    }

    QList<QueryMatch> matches;

    if (term == "connect")
    {
        QueryMatch match(this);

        match.setType(QueryMatch::ExactMatch);
        match.setIcon(KIcon("user-"));
        match.setText(i18n("Set all accounts as online"));
        match.setData("connect");

        matches.append(match);
    }
    else if (term == "disconnect")
    {
        QueryMatch match(this);

        match.setType(QueryMatch::ExactMatch);
        match.setIcon(KIcon("user-offline"));
        match.setText(i18n("Set all accounts as offline"));
        match.setData("disconnect");

        matches.append(match);
    }
    else if (term.startsWith(QLatin1String("status")))
    {
        QStringList query = context.query().split(' ');
        // Take the status text
        query.takeFirst();
        if (!query.isEmpty())
        {
            // Take the status to set
            const QString status = query.takeFirst();
            if (!status.isEmpty())
            {
                QueryMatch match(this);

                match.setType(QueryMatch::ExactMatch);
                match.setIcon(KIcon("user-away"));
                match.setText(i18nc("The \': \' is used as a separator", "Status: %1", status));
                // Rejoin the status message
                const QString message = query.join(" ");
                if (!message.isEmpty())
                    match.setSubtext(i18nc("The \': \' is used as a separator", "Message: %1", message));
                match.setData("status");

                matches.append(match);
            }
        }
    }
    else if (term.startsWith(QLatin1String("message")))
    {
        QStringList query = context.query().split(' ');
        // Take the status text
        query.takeFirst();
        if (!query.isEmpty())
        {
            // Rejoin the rest of the message
            const QString message = query.join(" ");
            if (!message.isEmpty())
            {
                QueryMatch match(this);

                match.setType(QueryMatch::ExactMatch);
                match.setIcon(KIcon("im-status-message-edit"));
                match.setText(i18nc("The \': \' is used as a separator", "Message: %1", message));
                match.setData(i18n("Set Status Message"));
                match.setData("status");

                matches.append(match);
            }
        }
    }
    QHashIterator<QString, QVariantMap> i(m_contactData);
    while (i.hasNext()) {
	i.next();
        // Keep a reference for easier use
        const ContactProperties& props = i.value();
        // Skip unreachable contacts
        if (!props["message_reachable"].toBool())
            continue;

        const QString name = props["display_name"].toString();
        const QString picture = props["picture"].toString();
        const QString status = props["status"].toString();
        const QString message = props["status_message"].toString();
        if (name.contains(term, Qt::CaseInsensitive))
        {
            QueryMatch match(this);

            match.setType((name.compare(context.query(), Qt::CaseInsensitive)) ? QueryMatch::PossibleMatch : QueryMatch::ExactMatch);
            match.setIcon(KIcon(KUrl(picture).isLocalFile() ? picture : "kopete"));
            match.setText(i18n("Send message to %1", name));
            const QString statusLine = i18n("Status: %1", status);
            const QString subtext = message.isEmpty() ? statusLine : i18n("%1\nMessage: %2", statusLine, message);
            match.setSubtext(subtext);
            match.setData(i.key());

            matches.append(match);
        }
    }

    context.addMatches(term, matches);
}

void KopeteRunner::run(const Plasma::RunnerContext& context, const Plasma::QueryMatch& match)
{
    Q_UNUSED(context)

    // HACK: Strip off the "kopete_" prefix
    const QString id = match.data().toString();
    QString method;
    QVariantList args;
    if (id == "connect")
        method = "connectAll";
    else if (id == "disconnect")
        method = "disconnectAll";
    else if (id == "status")
    {
        method = "setOnlineStatus";
        QStringList status = match.text().split(": ");
        status.takeFirst();
        QStringList message = match.subtext().split(": ");
        message.takeFirst();
        args << status.join(": ") << message.join(": ");
    }
    else if (id == "message")
    {
        method = "setStatusMessage";
        QStringList message = match.text().split(": ");
        message.takeFirst();
        args << message.join(": ");
    }
    else if (!QUuid(id).isNull())
    {
        method = "openChat";
        args << id;
    }
    else
        qDebug("Unknown ID: %s", id.toUtf8().constData());
    if (!method.isNull())
    {
        QDBusMessage message = generateMethodCall(method);
        message.setArguments(args);
        QDBusConnection::sessionBus().send(message);
    }
}

void KopeteRunner::loadData()
{
    if (m_checkLoaded) {
        return;
    }


    m_checkLoaded = true;
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(KopeteDBusService);
    m_loaded = reply.isValid() && reply.value();
    //kDebug() << "**************************" << m_loaded;
#if 0
    // Request contacts
    QDBusMessage request = generateMethodCall("contacts");
    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(request);
    // If the request succeeded
    if (reply.isValid())
    {
        // Mark that we are loading
        m_loaded = true;
        // Get the contact list
        const QStringList& contacts = reply.value();

        // Fetch data each time a contact status changes
        QDBusConnection::sessionBus().connect(KopeteDBusService, KopeteDBusPath, KopeteDBusInterface,
                                              "contactChanged", "s", this, SLOT(updateContact(QString)));
        // Update each one
        // THIS IS BROKEN

        // do not make one call per contact
        // this makes over a thousand calls on my system
        // please modify Kopete to transfer all information in one call

        // Alternatively, cache this data. On my system, this causes
        // kopete to spike to 100% CPU usage for between 5 to 10
        // seconds every time I press Alt+F2 (or Esc in krunner).
        // -thiago

        foreach (const QString& contact, contacts)
            updateContact(contact);
    }
#endif
}

void KopeteRunner::slotPrepare()
{
    //kDebug();
    // Refresh the data
    loadData();
}

void KopeteRunner::slotTeardown()
{
    // Don't get updated when not needed
#if 0
    QDBusConnection::sessionBus().disconnect(KopeteDBusService, KopeteDBusPath, KopeteDBusInterface, "contactChanged", this, SLOT(updateContact(QString)));
#endif
    //kDebug();
    m_contactData.clear();
    m_checkLoaded = false;
}

void KopeteRunner::updateContact(const QString& uuid)
{
    QDBusMessage message = generateMethodCall("contactProperties");
    message << uuid;
    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);

    if (reply.isValid())
        m_contactData[uuid] = reply.value();
}

#include <kopeterunner.moc>
