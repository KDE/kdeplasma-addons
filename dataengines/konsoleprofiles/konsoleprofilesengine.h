/*
 *  SPDX-FileCopyrightText: 2011 Shaun Reich <shaun.reich@kdemail.net>
 *  SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KONSOLEPROFILESENGINE_H
#define KONSOLEPROFILESENGINE_H

#include <Plasma/DataEngine>

class KDirWatch;

class KonsoleProfilesEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    explicit KonsoleProfilesEngine(QObject *parent, const QVariantList &args);
    ~KonsoleProfilesEngine() override;

    void init();
    Plasma::Service *serviceForSource(const QString &source) override;

private Q_SLOTS:
    void profilesChanged();

private:
    void loadProfiles();

    KDirWatch *m_dirWatch;
};

#endif
