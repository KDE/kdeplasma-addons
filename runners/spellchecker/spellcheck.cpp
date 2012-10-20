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
#include <KGlobal>
#include <KIcon>
#include <QSet>

SpellCheckRunner::SpellCheckRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    KGlobal::locale()->insertCatalog(QLatin1String( "krunner_spellcheckrunner" ));
    setObjectName(QLatin1String( "Spell Checker" ));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::SlowSpeed);
}

SpellCheckRunner::~SpellCheckRunner()
{
}

void SpellCheckRunner::init()
{
    Plasma::AbstractRunner::init();

    //Connect prepare and teardown signals
    connect(this, SIGNAL(prepare()), this, SLOT(loaddata()));
    connect(this, SIGNAL(teardown()), this, SLOT(destroydata()));
}

//Load a default dictionary and some locale names
void SpellCheckRunner::loaddata()
{
    //Load the default speller, with the default language
    if (!m_spellers.contains("")) {
        m_spellers[""] = QSharedPointer<Sonnet::Speller> (new Sonnet::Speller(""));
    }
    //store all language names, makes it possible to type "spell german TERM" if english locale is set
    //Need to construct a map between natual language names and names the spell-check recognises.
    KLocale *locale = KGlobal::locale();
    const QStringList avail = m_spellers[""]->availableLanguages();
    //We need to filter the available languages so that we associate the natural language
    //name (eg. 'german') with one sub-code.
    QSet<QString> families;
    //First get the families
    foreach (const QString &code, avail) {
        families +=code.left(2);
    }
    //Now for each family figure out which is the main code.
    foreach (const QString &fcode,families) {
        QStringList family = avail.filter(fcode);
        QString code;
        //If we only have one code, use it.
        //If a string is the default language, use it
        if (family.contains(m_spellers[""]->language())) {
            code = m_spellers[""]->language();
        } else if (fcode == QLatin1String("en")) {
            //If the family is english, default to en_US.
            if (family.contains("en_US")) {
                code = QLatin1String("en_US");
            }
        } else if (family.contains(fcode+QLatin1String("_")+fcode.toUpper())) {
            //If we have a speller of the form xx_XX, try that.
            //This gets us most European languages with more than one spelling.
            code =  fcode+QLatin1String("_")+fcode.toUpper();
        } else {
            //Otherwise, pick the first value as it is highest priority.
            code = family.first();
        }
        //Finally, add code to the map.
        const QString name = locale->languageCodeToName(fcode);
        if (!name.isEmpty()) {
            m_languages[name.toLower()] = code;
        }
//         kDebug() << "SPELL lang: " << fcode<< "::"<< name << "  :  " << code;
    }

}

void SpellCheckRunner::destroydata()
{
    //Clear the data arrays to save memory
    m_spellers.clear();
}

void SpellCheckRunner::reloadConfiguration()
{
    m_triggerWord = config().readEntry("trigger", i18n("spell"));
    //Processing will be triggered by "keyword "
    m_triggerWord += QLatin1Char( ' ' );

    m_requireTriggerWord = config().readEntry("requireTriggerWord", true);

    Plasma::RunnerSyntax s(i18nc("Spelling checking runner syntax, first word is trigger word, e.g.  \"spell\".",
                                 "%1:q:", m_triggerWord),
                           i18n("Checks the spelling of :q:."));

    if (!m_requireTriggerWord) {
        s.addExampleQuery(QLatin1String( ":q:" ));
    }

    QList<Plasma::RunnerSyntax> syns;
    syns << s;
    setSyntaxes(syns);
}

/* Take the input query, split into a list, and see if it contains a language to spell in.
 * Return the empty string if we can't match a language. */
QString SpellCheckRunner::findlang(const QStringList& terms)
{
    //If first term is a language code (like en_GB), set it as the spell-check language
    if (terms.count() >= 1 && m_spellers[""]->availableLanguages().contains(terms[0])) {
        return terms[0];
    }
    //If we have two terms and the first is a language name (eg 'french'),
    //set it as the available language
    else if (terms.count() >=2) {
        QString code;
        {
            //Is this a descriptive language name?
            QMap<QString, QString>::const_iterator it = m_languages.constFind(terms[0].toLower());
            if (it != m_languages.constEnd()) {
                code = *it;
            }
            //Maybe it is a subset of a language code?
            else {
                QStringList codes = QStringList(m_languages.values()).filter(terms[0]);
                if (!codes.isEmpty()) {
                    code = codes.first();
                }
            }
        }

        if (!code.isEmpty()) {
            //We found a valid language! Check still available
            const QStringList avail = m_spellers[""]->availableLanguages();
            //Does the spell-checker like it?
            if (avail.contains(code)) {
                return code;
            }
        }
        //FIXME: Support things like 'british english' or 'canadian french'
    }
    return QLatin1String("");
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

    //Pointer to speller object with our chosen language
    QSharedPointer<Sonnet::Speller> speller = m_spellers[""];

    if (speller->isValid()) {
        QStringList terms = query.split(' ', QString::SkipEmptyParts);
        QString lang = findlang(terms);
        //If we found a language, create a new speller object using it.
        if (!lang.isEmpty()) {
            //First term is the language
            terms.removeFirst();
            //New speller object if we don't already have one
            if (!m_spellers.contains(lang)) {
                QMutexLocker lock (&m_spellLock);
                //Check nothing happened while we were acquiring the lock
                if (!m_spellers.contains(lang)) {
                    m_spellers[lang] = QSharedPointer<Sonnet::Speller>(new Sonnet::Speller(lang));
                }
            }
            speller = m_spellers[lang];
            //Rejoin the strings
            query = terms.join(QLatin1String(" "));
        }
    }

    if (query.size() < 2) {
        return;
    }

    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::InformationalMatch);

    if (speller->isValid()) {
        QStringList suggestions;
        const bool correct = speller->checkAndSuggest(query,suggestions);
        if (correct) {
            match.setIcon(KIcon(QLatin1String( "checkbox" )));
            match.setText(i18n("Correct")+QLatin1String(": ")+query);
        } else {
            match.setIcon(KIcon(QLatin1String( "edit-delete" )));
            const QString recommended = i18n("Suggested words: %1", suggestions.join(i18nc("seperator for a list of words", ", ")));
            //TODO: try setting a text and a subtext, with the subtext being the suggestions
            match.setText(recommended);
            match.setData(suggestions);
        }
    } else {
        match.setIcon(KIcon(QLatin1String("task-attention")));
        match.setText(i18n("Could not find a dictionary."));
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
