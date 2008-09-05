/*
 *   Copyright 2008 Montel Laurent <montel@kde.org>
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

#ifndef KONQUERORSESSIONS_H
#define KONQUERORSESSIONS_H

#include <plasma/abstractrunner.h>

#include <KIcon>

class KonquerorSessions : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    KonquerorSessions( QObject *parent, const QVariantList& args );
    ~KonquerorSessions();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
protected slots:
    void loadSessions();
private:
    KIcon m_icon;
    QHash<QString, QString> m_sessions;
};

K_EXPORT_PLASMA_RUNNER(konquerorsessions, KonquerorSessions)

#endif
