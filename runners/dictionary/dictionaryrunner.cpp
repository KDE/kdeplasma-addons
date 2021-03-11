/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner.h"

#include <KLocalizedString>
#include <QEventLoop>
#include <QStringList>
#include <QTimer>

static const char CONFIG_TRIGGERWORD[] = "triggerWord";

DictionaryRunner::DictionaryRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : AbstractRunner(parent, metaData, args)
{
    m_engine = new DictionaryMatchEngine(m_consumer.dataEngine(QStringLiteral("dict")), this);

    setPriority(LowPriority);
    setObjectName(QLatin1String("Dictionary"));
}

void DictionaryRunner::init()
{
    reloadConfiguration();
}

void DictionaryRunner::reloadConfiguration()
{
    KConfigGroup c = config();
    m_triggerWord = c.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define"));
    if (!m_triggerWord.isEmpty()) {
        m_triggerWord.append(QLatin1Char(' '));
        setTriggerWords({m_triggerWord});
    } else {
        setMatchRegex(QRegularExpression());
    }
    setSyntaxes({Plasma::RunnerSyntax(i18nc("Dictionary keyword", "%1:q:", m_triggerWord), i18n("Finds the definition of :q:."))});
}

void DictionaryRunner::match(Plasma::RunnerContext &context)
{
    QString query = context.query();
    if (query.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        query.remove(0, m_triggerWord.length());
    }
    if (query.isEmpty()) {
        return;
    }
    QEventLoop loop;
    QTimer::singleShot(400, &loop, [&loop]() {
        loop.quit();
    });
    loop.exec();
    if (!context.isValid()) {
        return;
    }
    QString returnedQuery = m_engine->lookupWord(query);
    if (!context.isValid()) {
        return;
    }

    static const QRegExp removeHtml(QLatin1String("<[^>]*>"));
    QString definitions(returnedQuery);
    definitions.remove(QLatin1Char('\r')).remove(removeHtml);
    while (definitions.contains(QLatin1String("  "))) {
        definitions.replace(QLatin1String("  "), QLatin1String(" "));
    }
    QStringList lines = definitions.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (lines.length() < 2) {
        return;
    }
    lines.removeFirst();

    QList<Plasma::QueryMatch> matches;
    int item = 0;
    static const QRegExp partOfSpeech(QLatin1String("(?: ([a-z]{1,5})){0,1} [0-9]{1,2}: (.*)"));
    QString lastPartOfSpeech;
    for (const QString &line : qAsConst(lines)) {
        if (partOfSpeech.indexIn(line) == -1) {
            continue;
        }
        if (!partOfSpeech.cap(1).isEmpty()) {
            lastPartOfSpeech = partOfSpeech.cap(1);
        }
        Plasma::QueryMatch match(this);
        match.setMultiLine(true);
        match.setText(lastPartOfSpeech + QLatin1String(": ") + partOfSpeech.cap(2));
        match.setRelevance(1 - (static_cast<double>(++item) / static_cast<double>(lines.length())));
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIconName(QStringLiteral("accessories-dictionary"));
        matches.append(match);
    }
    context.addMatches(matches);
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(DictionaryRunner, "plasma-runner-dictionary.json")

#include "dictionaryrunner.moc"
