/*
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KONSOLEPROFILES_H
#define KONSOLEPROFILES_H

#include <KRunner/AbstractRunner>

class KDirWatch;

struct KonsoleProfileData {
    QString displayName;
    QString iconName;
};

Q_DECLARE_TYPEINFO(KonsoleProfileData, Q_MOVABLE_TYPE);

class KonsoleProfiles : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit KonsoleProfiles(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KonsoleProfiles() override;

    void init() override;
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

private Q_SLOTS:
    void loadProfiles();

private:
    KDirWatch *m_profileFilesWatch = nullptr;
    QList<KonsoleProfileData> m_profiles;
    QLatin1String m_triggerWord = QLatin1String("konsole");
};

#endif
