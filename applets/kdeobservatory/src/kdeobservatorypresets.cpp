/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "kdeobservatorypresets.h"

#include <KDebug>
#include <KLocale>

QList<QStringList> KdeObservatoryPresets::presets;

void KdeObservatoryPresets::init(const QString &data)
{
    if (!data.isEmpty())
        foreach (QString row, data.split('\n'))
            if (!row.isEmpty())
            {
                QStringList list;
                foreach (QString token, row.split(';'))
                        list << token;
                presets << list;
            }
}

QStringList KdeObservatoryPresets::preset(PresetInfo info)
{
    QStringList list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << presets.at(i).at(info);
    return list;
}

QList<bool> KdeObservatoryPresets::automaticallyInViews()
{
    QList<bool> list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << ((presets.at(i).last() == "false") ? false:true);
    return list;
}

QStringList KdeObservatoryPresets::viewsPreset()
{
    return QStringList() << i18n("Top Active Projects") << i18n("Commit History") << i18n("Top Developers") << i18n("Krazy Report");
}

QList<bool> KdeObservatoryPresets::viewsActivePreset()
{
    return QList<bool>() << true << true << true << true;
}
