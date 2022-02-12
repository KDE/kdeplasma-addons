/*
 *   SPDX-FileCopyrightText: 2008 Sebastian Kügler <sebas@kde.org>
 *   SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATESESSIONS_H
#define KATESESSIONS_H

#include <KRunner/AbstractRunner>

class KDirWatch;

using namespace Plasma;

class KateSessions : public AbstractRunner
{
    Q_OBJECT

public:
    explicit KateSessions(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KateSessions() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private Q_SLOTS:
    QStringList loadSessions();

private:
    QString m_sessionsFolderPath;
    const QLatin1String m_triggerWord = QLatin1String("kate");
};

#endif
