/*
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KRunner/AbstractRunner>

using namespace KRunner;

class ColorsRunner : public AbstractRunner
{
    Q_OBJECT

public:
    ColorsRunner(QObject *parent, const KPluginMetaData &metaData);
    ~ColorsRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;
    QMimeData *mimeDataForMatch(const QueryMatch &match) override;
};
