/*
 *   Copyright (C) 2007 Teemu Rytilahti <tpr@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "contactsrunner.h"

#include <QPixmap>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KToolInvocation>
/*#include <KMimeType>
#include <KUrl>

*/

#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kabc/picture.h>

#include "imageiconengine.h"

ContactsRunner::ContactsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName("Contacts");

    m_book = KABC::StdAddressBook::self(true /*asynchronous*/);

    m_icon = KIcon("internet-mail");
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds people in your address book matching :q:.")));
}

ContactsRunner::~ContactsRunner()
{
}

void ContactsRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query().toLower();

    if (term.size() < 3) {
        return;
    }

    QList<Plasma::QueryMatch> matches;

    foreach (const KABC::Addressee &a, m_book->allAddressees()) {
        if (!context.isValid()) {
            return;
        }

        const bool matchedName = a.realName().contains(term, Qt::CaseInsensitive);
        bool matchedMail = false;

        if (!matchedName) {
            // Name didn't match, so lets try the name portion of the email address
            const int indexOf = a.preferredEmail().indexOf(term, Qt::CaseInsensitive);
            matchedMail = indexOf > -1 && indexOf < a.preferredEmail().indexOf('@');
        }

        if (matchedName || matchedMail) {
            //kDebug() << "Possible match: " << a.realName() << " <" << a.preferredEmail() << ">";
            Plasma::QueryMatch match(this);

            //TODO: exact match if the name is exact? =)
            match.setType(Plasma::QueryMatch::PossibleMatch);

            if(!a.photo().isEmpty()) {
                /* TODO it isn't anymore possible to use url as a photo for an user?
                   if(a.photo().isIntern()) {
                  }
                  else {
                      KURL url(a.photo().url());
                      kDebug() << "photo url: " << url.prettyUrl() << endl;
                      if(url.isLocalFile()) {
                        icon = QIcon(url);
                    }
                  }*/
                QIcon icon(new ImageIconEngine(a.photo().data()));
                match.setIcon(icon);
            } else {
                match.setIcon(m_icon);
            }

            match.setText(i18nc("Open the default mail program to mail someone", "Mail to %1", a.realName()));

            if (!a.realName().isEmpty()) {
                match.setData(a.realName() + '<' + a.preferredEmail() + '>');
            } else {
                match.setData(a.preferredEmail());
            }
            matches.append(match);
        }
    }

    context.addMatches(term, matches);
}

void ContactsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    const QString address = match.data().toString();
    const QString name = match.text();

    kDebug() << "run name '" << name << "' with address '" << address << "'";

    KToolInvocation::invokeMailer(address);
}

#include "contactsrunner.moc"
