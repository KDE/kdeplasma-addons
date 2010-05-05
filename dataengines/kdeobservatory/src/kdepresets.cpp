/*************************************************************************
 * Copyright 2009-2010 Sandro Andrade sandroandrade@kde.org              *
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

#include "kdepresets.h"

#include <KLocale>

QList<QStringList> KdePresets::presets;

void KdePresets::init(const QString &data)
{
    if (!data.isEmpty())
    {
        foreach (const QString &row, data.split('\n'))
        {
            if (!row.isEmpty())
            {
                QStringList list;
                foreach (const QString &token, row.split(';'))
                {
                    QString newToken = token;
                    list << newToken.remove('\r');
                }
                presets << list;
            }
        }
    }
}

QStringList KdePresets::preset(PresetInfo info)
{
    QStringList list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << presets.at(i).at(info);
    return list;
}

QList<QVariant> KdePresets::automaticallyInViews()
{
    QList<QVariant> list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << ((presets.at(i).last() == "false") ? false:true);

    return list;
}

QStringList KdePresets::viewsPreset()
{
    return QStringList() << i18n("Top Active Projects") << i18n("Commit History") << i18n("Top Developers") << i18n("Krazy Report");
}

QList<QVariant> KdePresets::viewsActivePreset()
{
    return QList<QVariant>() << true << true << true << true;
}
