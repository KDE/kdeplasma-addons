/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
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

#ifndef CURRENCY_H
#define CURRENCY_H

#include "unitcategory.h"

class Currency : public Conversion::UnitCategory
{
public:
    enum Id {
        Eur = 8000, Ats, Bef, Nlg, Fim, Frf, Dem, Iep, Itl, Luf, Pte, Esp, Grd, Sit, Cyp, Mtl, Skk,
        Usd, Jpy, Bgn, Czk, Dkk, Eek, Gbp, Huf, Ltl, Lvl, Pln, Ron, Sek, Chf, Nok, Hrk, Rub, Try,
        Aud, Brl, Cad, Cny, Hkd, Idr, Inr, Krw, Mxn, Myr, Nzd, Php, Sgd, Thb, Zar
    };

    Currency(QObject* parent = 0);
    virtual Conversion::Value convert(const Conversion::Value& value,
                                      const Conversion::Unit* to);

private:
    QString m_cache;
    bool m_update;
};

#endif
