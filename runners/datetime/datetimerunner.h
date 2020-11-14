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

/**
 * This class looks for matches in the set of .desktop files installed by
 * applications. This way the user can type exactly what they see in the
 * applications menu and have it start the appropriate app. Essentially anything
 * that KService knows about, this runner can launch
 */

class DateTimeRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~DateTimeRunner() override;

    void match(Plasma::RunnerContext &context) override;

private:
    QHash<QString, QDateTime> datetime(const QStringRef &tz);
    void addMatch(const QString &text, const QString &clipboardText,
                  Plasma::RunnerContext &context, const QString& iconName);
};

#endif

