/*
 *   Copyright (C) 2007 Ryan P. Bitanga <ephebiphobic@gmail.com>
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

#include "spellcheck.h"

#include <QClipboard>

#include <KApplication>
// #include <KDebug>
#include <KIcon>

SpellCheckRunner::SpellCheckRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    KGlobal::locale()->insertCatalog("krunner_spellcheckrunner");
    setObjectName("Spell Checker");
    setSpeed(AbstractRunner::SlowSpeed);

    reloadConfiguration();
}

SpellCheckRunner::~SpellCheckRunner()
{
}

void SpellCheckRunner::reloadConfiguration()
{
    m_triggerWord = config().readEntry("trigger", i18n("spell"));
    //Processing will be triggered by "keyword "
    m_triggerWord += ' ';

    m_requireTriggerWord = config().readEntry("requireTriggerWord", true);

    Plasma::RunnerSyntax s(i18nc("Spelling checking runner syntax, first word is trigger word, e.g.  \"spell\".",
                                 "%1:q:", m_triggerWord),
                           i18n("Checks the spelling of :q:."));

    if (!m_requireTriggerWord) {
        s.addExampleQuery(":q:");
    }

    QList<Plasma::RunnerSyntax> syns;
    syns << s;
    setSyntaxes(syns);
}

void SpellCheckRunner::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) {
        return;
    }


    const QString term = context.query();
    QString query = term;

    if (m_requireTriggerWord) {
        int len = m_triggerWord.length();
        if (query.left(len) != m_triggerWord) {
            return;
        }

        query = query.mid(len).trimmed();
    }

    if (query.size() < 3) {
        return;
    }

    QStringList suggestions;
    const bool correct = m_speller.checkAndSuggest(query,suggestions);

    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::InformationalMatch);

    if (correct) {
        match.setIcon(KIcon("checkmark"));
        match.setText(i18n("Correct"));
    } else {
        match.setIcon(KIcon("no"));
        const QString recommended = i18n("Suggested words: %1", suggestions.join (", "));
        //TODO: try setting a text and a subtext, with the subtext being the suggestions
        match.setText(recommended);
        match.setData(suggestions);
    }

    context.addMatch(term, match);
}

void SpellCheckRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    //Copy words to clipboard
    kapp->clipboard()->setText(match.data().toString());
}

#include "spellcheck.moc"
