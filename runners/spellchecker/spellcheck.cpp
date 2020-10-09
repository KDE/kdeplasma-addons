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

#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>
#include <QLocale>
#include <QSet>
#include <QIcon>
#include <QMimeData>

#include <KLocalizedString>

SpellCheckRunner::SpellCheckRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName(QStringLiteral("Spell Checker"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::SlowSpeed);
}

SpellCheckRunner::~SpellCheckRunner() = default;

void SpellCheckRunner::init()
{
    m_actions = {addAction(QStringLiteral("copyToClipboard"),
                           QIcon::fromTheme(QStringLiteral("edit-copy")),
                           i18nc("@action", "Copy to Clipboard"))};

    //Connect prepare and teardown signals
    connect(this, &SpellCheckRunner::prepare, this, &SpellCheckRunner::loadData);
    connect(this, &SpellCheckRunner::teardown, this, &SpellCheckRunner::destroydata);

    reloadConfiguration();
}

//Load a default dictionary and some locale names
void SpellCheckRunner::loadData()
{
    //Load the default speller, with the default language
    auto defaultSpellerIt = m_spellers.find(QString());
    if (defaultSpellerIt == m_spellers.end()) {
        defaultSpellerIt = m_spellers.insert(QString(), QSharedPointer<Sonnet::Speller> (new Sonnet::Speller(QString())));
    }
    auto& defaultSpeller = defaultSpellerIt.value();

    //store all language names, makes it possible to type "spell german TERM" if english locale is set
    //Need to construct a map between natual language names and names the spell-check recognises.
    const QStringList avail = defaultSpeller->availableLanguages();
    //We need to filter the available languages so that we associate the natural language
    //name (eg. 'german') with one sub-code.
    QSet<QString> families;
    //First get the families
    for (const QString &code: avail) {
        families +=code.left(2);
    }
    //Now for each family figure out which is the main code.
    for (const QString &fcode: qAsConst(families)) {
        const QStringList family = avail.filter(fcode);
        QString code;
        //If we only have one code, use it.
        //If a string is the default language, use it
        if (family.contains(defaultSpeller->language())) {
            code = defaultSpeller->language();
        } else if (fcode == QLatin1String("en")) {
            //If the family is english, default to en_US.
            const auto enUS = QStringLiteral("en_US");
            if (family.contains(enUS)) {
                code = enUS;
            }
        } else if (family.contains(fcode+QLatin1Char('_')+fcode.toUpper())) {
            //If we have a speller of the form xx_XX, try that.
            //This gets us most European languages with more than one spelling.
            code =  fcode+QLatin1Char('_')+fcode.toUpper();
        } else {
            //Otherwise, pick the first value as it is highest priority.
            code = family.first();
        }
        //Finally, add code to the map.
        // FIXME: We need someway to map languageCodeToName
        const QString name;// = locale->languageCodeToName(fcode);
        if (!name.isEmpty()) {
            m_languages[name.toLower()] = code;
        }
    }

}

void SpellCheckRunner::destroydata()
{
    //Clear the data arrays to save memory
    m_spellers.clear();
}

void SpellCheckRunner::reloadConfiguration()
{
    const KConfigGroup cfg = config();
    m_triggerWord = cfg.readEntry("trigger", i18n("spell"));
    //Processing will be triggered by "keyword "
    m_requireTriggerWord = cfg.readEntry("requireTriggerWord", true) && !m_triggerWord.isEmpty();
    m_triggerWord += QLatin1Char( ' ' );

    Plasma::RunnerSyntax s(i18nc("Spelling checking runner syntax, first word is trigger word, e.g.  \"spell\".",
                                 "%1:q:", m_triggerWord), i18n("Checks the spelling of :q:."));

    if (!m_requireTriggerWord) {
        s.addExampleQuery(QStringLiteral(":q:"));
    }

    setSyntaxes({s});
}

/* Take the input query, split into a list, and see if it contains a language to spell in.
 * Return the empty string if we can't match a language. */
QString SpellCheckRunner::findLang(const QStringList& terms)
{
    const auto &defaultSpeller = m_spellers[QString()];
    //If first term is a language code (like en_GB), set it as the spell-check language
    if (!terms.isEmpty() && defaultSpeller->availableLanguages().contains(terms[0])) {
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
            const QStringList avail = defaultSpeller->availableLanguages();
            //Does the spell-checker like it?
            if (avail.contains(code)) {
                return code;
            }
        }
        //FIXME: Support things like 'british english' or 'canadian french'
    }
    return QString();
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
    QSharedPointer<Sonnet::Speller> speller = m_spellers[QString()];

    if (speller->isValid()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QStringList terms = query.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
        QStringList terms = query.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif
        const QString lang = findLang(terms);
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
            query = terms.join(QLatin1Char(' '));
        }
    }

    if (query.size() < 2) {
        return;
    }

    if (speller->isValid()) {
        QStringList suggestions;
        const bool correct = speller->checkAndSuggest(query,suggestions);
        if (correct) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::ExactMatch);
            match.setIconName(QStringLiteral("checkbox"));
            match.setText(query);
            match.setSubtext(i18nc("Term is spelled correctly", "Correct"));
            match.setData(query);
            context.addMatch(match);
        } else {
            for (const auto& suggestion : qAsConst(suggestions)) {
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setIconName(QStringLiteral("edit-rename"));
                match.setText(suggestion);
                match.setSubtext(i18n("Suggested term"));
                match.setData(suggestion);
                context.addMatch(match);
            }
        }
    } else {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIconName(QStringLiteral("data-error"));
        match.setText(i18n("No dictionary found, please install hspell"));
        context.addMatch(match);
    }
}

void SpellCheckRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    QGuiApplication::clipboard()->setText(match.data().toString());
}

QList<QAction *> SpellCheckRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)

    return m_actions;
}

QMimeData * SpellCheckRunner::mimeDataForMatch(const Plasma::QueryMatch &match)
{
    QMimeData *result = new QMimeData();
    const QString text = match.data().toString();
    result->setText(text);
    return result;
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(SpellCheckRunner, "plasma-runner-spellchecker.json")

#include "spellcheck.moc"
