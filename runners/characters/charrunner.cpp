/* Copyright 2010  Anton Kreuzkamp <akreuzkamp@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "charrunner.h"

#include <KIcon>

#include <Plasma/QueryMatch>

CharacterRunner::CharacterRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName(QLatin1String( "CharacterRunner" ));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                         Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
                         Plasma::RunnerContext::ShellCommand);
    reloadConfiguration();
}

CharacterRunner::~CharacterRunner()
{
}

void CharacterRunner::reloadConfiguration()
{
  KConfigGroup grp = config(); //Create config-object

  m_triggerWord = grp.readEntry(CONFIG_TRIGGERWORD, "#"); //read out the triggerword
  m_aliases = grp.readEntry(CONFIG_ALIASES, QStringList());
  m_codes = grp.readEntry(CONFIG_CODES, QStringList());
  addSyntax(Plasma::RunnerSyntax(m_triggerWord + QLatin1String( ":q:" ),
                                 i18n("Creates Characters from :q: if it is a hexadecimal code or defined alias.")));
}

void CharacterRunner::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    QString specChar;

    term = term.replace(QLatin1Char( ' ' ), QLatin1String( "" )); //remove blanks
    if (term.length() < 2) //ignore too short queries
    {
        return;
    }
    if (!term.startsWith(m_triggerWord)) //ignore queries without the triggerword
    {
      return;
    }
    term = term.remove(0, m_triggerWord.length()); //remove the triggerword

    if (m_aliases.contains(term)) //replace aliases by their hex.-code
    {
      term = m_codes[m_aliases.indexOf(term)];
    }

    bool ok; //checkvariable
    int hex = term.toInt(&ok, 16); //convert query into int
    if (!ok) //check if conversion was successful
    {
      return;
    }

    //make special caracter out of the hex.-code
    specChar=QString();
    specChar.toUtf8();
    specChar[0]=hex;

    //create match
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::InformationalMatch);
    match.setIcon(KIcon(QLatin1String( "accessories-character-map" )));
    match.setText(specChar);
    match.setData(specChar);
    match.setId(QString());
    context.addMatch(term, match);
}

#include "charrunner.moc"
