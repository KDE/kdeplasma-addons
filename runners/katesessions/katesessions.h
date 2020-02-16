/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *   Copyright 2020  Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KATESESSIONS_H
#define KATESESSIONS_H

#include <KRunner/AbstractRunner>

class KDirWatch;

class KateSessions : public Plasma::AbstractRunner {
    Q_OBJECT

    public:
        explicit KateSessions(QObject *parent, const QVariantList& args);
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
