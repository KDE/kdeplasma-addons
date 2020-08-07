/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner.h"

#include <QStringList>
#include <KLocalizedString>

static const char CONFIG_TRIGGERWORD[] = "triggerWord";

DictionaryRunner::DictionaryRunner(QObject *parent, const QVariantList &args)
    : AbstractRunner(parent, args)
{
    m_engine = new DictionaryMatchEngine(dataEngine(QStringLiteral("dict")), this);

    setSpeed(SlowSpeed);
    setPriority(LowPriority);
    setObjectName(QLatin1String("Dictionary"));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
            Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
            Plasma::RunnerContext::ShellCommand);
}

void DictionaryRunner::init()
{
    reloadConfiguration();
}

void DictionaryRunner::reloadConfiguration()
{
    KConfigGroup c = config();
    m_triggerWord = c.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define"));
    if (!m_triggerWord.isEmpty())
        m_triggerWord.append(QLatin1Char(' '));
    setSyntaxes(QList<Plasma::RunnerSyntax>() << Plasma::RunnerSyntax(Plasma::RunnerSyntax(i18nc("Dictionary keyword", "%1:q:", m_triggerWord), i18n("Finds the definition of :q:."))));
}

void DictionaryRunner::match(Plasma::RunnerContext &context)
{
    QString query = context.query();
    if (!query.startsWith(m_triggerWord, Qt::CaseInsensitive))
        return;
    query.remove(0, m_triggerWord.length());
    if (query.isEmpty())
        return;
    QString returnedQuery = m_engine->lookupWord(query);

    if (!context.isValid())
        return;

    static const QRegExp removeHtml(QLatin1String("<[^>]*>"));
    QString definitions(returnedQuery);
    definitions.remove(QLatin1Char('\r')).remove(removeHtml);
    while (definitions.contains(QLatin1String("  ")))
        definitions.replace(QLatin1String("  "), QLatin1String(" "));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList lines = definitions.split(QLatin1Char('\n'), QString::SkipEmptyParts);
#else
    QStringList lines = definitions.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
#endif
    if (lines.length() < 2)
        return;
    lines.removeFirst();

    QList<Plasma::QueryMatch> matches;
    int item = 0;
    static const QRegExp partOfSpeech(QLatin1String("(?: ([a-z]{1,5})){0,1} [0-9]{1,2}: (.*)"));
    QString lastPartOfSpeech;
    foreach (const QString &line, lines) {
        if (partOfSpeech.indexIn(line) == -1)
            continue;
        if (!partOfSpeech.cap(1).isEmpty())
            lastPartOfSpeech = partOfSpeech.cap(1);
        Plasma::QueryMatch match(this);
        match.setText(query + QLatin1String(": ") + lastPartOfSpeech);
        match.setRelevance(1 - (static_cast<double>(++item) / static_cast<double>(lines.length())));
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIconName(QStringLiteral("accessories-dictionary"));
        match.setSubtext(partOfSpeech.cap(2));
        matches.append(match);
    }
    context.addMatches(matches);
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(DictionaryRunner, "plasma-runner-dictionary.json")

#include "dictionaryrunner.moc"
