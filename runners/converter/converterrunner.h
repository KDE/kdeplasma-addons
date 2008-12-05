/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONVERTERRUNNER_H
#define CONVERTERRUNNER_H

#include <plasma/abstractrunner.h>

/**
 * This class converts values to different units.
 */
class ConverterRunner : public Plasma::AbstractRunner
{
Q_OBJECT

public:
    ConverterRunner(QObject* parent, const QVariantList &args);
    ~ConverterRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
};

K_EXPORT_PLASMA_RUNNER(converterrunner, ConverterRunner)

#endif
