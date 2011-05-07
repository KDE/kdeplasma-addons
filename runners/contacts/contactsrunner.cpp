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


#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KToolInvocation>
#include <akonadi/contact/contactdefaultactions.h>
/*#include <KMimeType>

*/

#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kabc/phonenumber.h>
#include <kabc/picture.h>

#include "imageiconengine.h"

ContactsRunner::ContactsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String( "Contacts" ));

    m_book = KABC::StdAddressBook::self(true /*asynchronous*/);

    m_icon = KIcon(QLatin1String( "internet-mail" ));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::NetworkLocation);
    addSyntax(Plasma::RunnerSyntax(QLatin1String( ":q:" ), i18n("Finds people in your address book matching :q:.")));
    setDefaultSyntax(Plasma::RunnerSyntax(i18nc("list of all people in address book", "contacts"),
                                   i18n("List all people in your address book.")));
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

        bool matchedName;
        bool matchedMail;

        if (term.compare(i18nc("list of all people in address book", "contacts"), Qt::CaseInsensitive) == 0 || term.compare(QLatin1String( "contacts" ), Qt::CaseInsensitive) == 0) {
            matchedName = true;
            matchedMail = true;
        } else {
            matchedName = a.realName().contains(term, Qt::CaseInsensitive);
            matchedMail = false;
        }

        if (!matchedName && !matchedMail) {
            // Name didn't match, so lets try the name portion of the email address
            const int indexOf = a.preferredEmail().indexOf(term, Qt::CaseInsensitive);
            matchedMail = indexOf > -1 && indexOf < a.preferredEmail().indexOf(QLatin1Char( '@' ));
        }

        if ((matchedName || matchedMail) && !a.preferredEmail().isEmpty()) {
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
                match.setData(QString(a.realName() + QLatin1String( " <" ) + a.preferredEmail() + QLatin1Char( '>' )));
            } else {
                match.setData(a.preferredEmail());
            }
            matches.append(match);
        }

        if (matchedName) {
            KABC::PhoneNumber::TypeList list;
            list << KABC::PhoneNumber::Home
                 << KABC::PhoneNumber::Work
                 << KABC::PhoneNumber::Voice
                 << KABC::PhoneNumber::Cell
                 << KABC::PhoneNumber::Video
                 << KABC::PhoneNumber::Car;
            foreach (const KABC::PhoneNumber::Type type, list) {
                KABC::PhoneNumber phonenumber = a.phoneNumber(type);
                if (!phonenumber.isEmpty()) {
                    //kDebug() << "Possible match: " << a.realName() << " tel: " << phonenumber.number();
                    Plasma::QueryMatch match(this);

                    //TODO: exact match if the name is exact? =)
                    match.setType(Plasma::QueryMatch::PossibleMatch);

                    if (!a.photo().isEmpty()) {
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

                    match.setText(i18nc("Open the default telephone program to call someone", "Call %1 at %2", a.realName(), phonenumber.typeLabel()));
                    match.setSubtext(phonenumber.number());
                    match.setData(QString(QLatin1String("call:") + phonenumber.number()));
                    matches.append(match);
                }
            }
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

    if (address.startsWith(QLatin1String("call:"))) {
	KABC::PhoneNumber number;
	number.setNumber(address.mid(5));
	Akonadi::ContactDefaultActions actions;
	actions.dialPhoneNumber( number );
    } else {
	KToolInvocation::invokeMailer(address);
    }
}

#include "contactsrunner.moc"
