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
#include <QDateTime>

using namespace Plasma;

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~DateTimeRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QHash<QString, QDateTime> datetimeAt(const QStringRef &zoneTerm, const QDateTime referenceTime = QDateTime::currentDateTime());
#else
    QHash<QString, QDateTime> datetimeAt(const QStringView &zoneTerm, const QDateTime referenceTime = QDateTime::currentDateTime());
#endif
    void addMatch(const QString &text, const QString &clipboardText, const qreal &relevance, const QString &iconName, RunnerContext &context);
};

#endif
