/*
    Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

// KF
#include <KConfigGroup>
#include <QDateTime>

class ConfigStorage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLunarPhaseShown MEMBER m_lunarPhaseShown NOTIFY lunarPhaseShownChanged)
    Q_PROPERTY(bool isSeasonShown MEMBER m_seasonShown NOTIFY seasonShownChanged)

public:
    explicit ConfigStorage(QObject *parent = nullptr);

    Q_INVOKABLE void save();

Q_SIGNALS:
    void lunarPhaseShownChanged();
    void seasonShownChanged();

private:
    KConfigGroup m_generalConfigGroup;

    bool m_lunarPhaseShown;
    bool m_seasonShown;
};

#endif
