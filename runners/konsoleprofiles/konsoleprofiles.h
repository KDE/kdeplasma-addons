/*
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KONSOLEPROFILES_H
#define KONSOLEPROFILES_H

#include "profilesmodel.h"
#include <KRunner/AbstractRunner>

using namespace Plasma;

class KonsoleProfiles : public AbstractRunner
{
    Q_OBJECT

public:
    explicit KonsoleProfiles(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private:
    ProfilesModel m_model;
    const QLatin1String m_triggerWord = QLatin1String("konsole");
};

#endif
