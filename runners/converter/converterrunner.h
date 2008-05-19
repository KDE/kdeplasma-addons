/*
 *   Copyright (C) 2007 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef CONVERTRUNNER_H
#define CONVERTRUNNER_H

#include <KGenericFactory>
#include <plasma/abstractrunner.h>
#include "unit.h"

/**
 * This class converts values to differemt units.
 */
class ConverterRunner : public Plasma::AbstractRunner
{
Q_OBJECT

public:
    ConverterRunner(QObject* parent, const QVariantList &args);
    ~ConverterRunner();

    void match(Plasma::RunnerContext &context);
    void exec(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

private:
    Unit* unitType(const QString& type);
    QList<Unit*> m_units;
};

K_EXPORT_PLASMA_RUNNER(converterrunner, ConverterRunner)

#endif
