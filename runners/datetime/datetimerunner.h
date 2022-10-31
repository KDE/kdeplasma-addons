/*
 *   SPDX-FileCopyrightText: 2010 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef DATETIMERUNNER_H
#define DATETIMERUNNER_H

#include <QDateTime>

#include <KRunner/AbstractRunner>
#include <KRunner/QueryMatch>

using namespace Plasma;

/**
 * This class looks for matches in the set of .desktop files installed by
 * applications. This way the user can type exactly what they see in the
 * applications menu and have it start the appropriate app. Essentially anything
 * that KService knows about, this runner can launch
 */

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~DateTimeRunner() override;

    void match(RunnerContext &context) override;

private:
    QHash<QString, QTimeZone> systemTimeZone();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QHash<QString, QTimeZone> matchingTimeZones(const QStringRef &searchTerm, const QDateTime &atDateTime = QDateTime());
#else
    QHash<QString, QTimeZone> matchingTimeZones(const QStringView &searchTerm, const QDateTime &atDateTime = QDateTime());
#endif
    void addMatch(const QString &text, const QString &clipboardText, RunnerContext &context, const QString &iconName);
};

#endif
