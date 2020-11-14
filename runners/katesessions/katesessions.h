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

class KateSessions : public Plasma::AbstractRunner {
    Q_OBJECT

    public:
        explicit KateSessions(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
        ~KateSessions() override;

        void match(Plasma::RunnerContext &context) override;
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    private Q_SLOTS:
        void loadSessions();

    private:
        KDirWatch* m_sessionWatch = nullptr;
        QString m_sessionsFolderPath;
        QStringList m_sessions;
        const QLatin1String m_triggerWord = QLatin1String("kate");
};

#endif
