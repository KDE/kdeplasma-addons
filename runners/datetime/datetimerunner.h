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

struct City {
    QString name;
    QString nameAscii;
    QByteArray timeZoneId;
};

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~DateTimeRunner() override;

    void match(RunnerContext &context) override;

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QHash<QString, QDateTime> datetime(const QStringRef &tz);
#else
    QHash<QString, QDateTime> datetime(const QStringView &tz);
#endif
    void addMatch(const QString &text, const QString &clipboardText, RunnerContext &context, const QString &iconName);
    void parseCityTZData();
    QList<City> cityTZData;
};

#endif
