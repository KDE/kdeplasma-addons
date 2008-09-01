/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
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

#ifndef BROWSERHISTORY_H
#define BROWSERHISTORY_H

#include <plasma/abstractrunner.h>

#include <KIcon>

class BrowserHistoryRunner : public Plasma::AbstractRunner {
    Q_OBJECT

    public:
        BrowserHistoryRunner( QObject *parent, const QVariantList& args );
        ~BrowserHistoryRunner();

        void match(Plasma::RunnerContext &context);
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

    private Q_SLOTS:
        void loadHistory();

    private:
        KIcon m_icon;
        QStringList m_history;
};

K_EXPORT_PLASMA_RUNNER(browserhistory, BrowserHistoryRunner)

#endif
