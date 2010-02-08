/*
 * Copyright 2009 Ben Boeckel <MathStuf@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KOPETERUNNER_H
#define KOPETERUNNER_H

// KDE includes
#include <KIcon>
#include <Plasma/AbstractRunner>

class KopeteRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        KopeteRunner(QObject* parent, const QVariantList& args);
        ~KopeteRunner();

        void match(Plasma::RunnerContext& context);
        void run(const Plasma::RunnerContext& context, const Plasma::QueryMatch& match);

    private:
        void loadData();

    private slots:
        void slotPrepare();
        void slotTeardown();
        void updateContact(const QString& uuid);

    private:
        typedef QVariantMap ContactProperties;
        typedef QHash<QString, ContactProperties> ContactData;

        ContactData m_contactData;
        bool m_loaded : 1;
        bool m_checkLoaded : 1;
};

K_EXPORT_PLASMA_RUNNER(kopete, KopeteRunner)

#endif
