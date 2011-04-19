/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*   Copyright 2003-2007  Niklas Knutsson <nq@altern.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
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

#ifndef QALCULATELABELS_H
#define QALCULATELABELS_H

#include <libqalculate/Calculator.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>

#include <QString>

class QalculateSettings;

class QalculateLabels
{
public:
    QalculateLabels(const QalculateSettings* qalculateSettings):
        m_qalculateSettings(qalculateSettings) {}
    QString drawStructure(MathStructure& m, const PrintOptions& po, InternalPrintStruct ips = top_ips);

private:
    const QalculateSettings *m_qalculateSettings;
    vector<MathStructure> result_parts;
    bool in_matrix;
};

#endif
