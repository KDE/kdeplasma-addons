/*
 *    Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *    Copyright 2016 Anthony Fieroni <bvbfan@abv.bg>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef MINIMIZEALL_HEADER
#define MINIMIZEALL_HEADER

#include <QObject>
#include <KWindowSystem>

class MinimizeAll : public QObject
{

Q_OBJECT
Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    MinimizeAll();
    ~MinimizeAll() override;

    Q_INVOKABLE void minimizeAllWindows();
    Q_INVOKABLE void unminimizeAllWindows();

    bool active() const;

public Q_SLOTS:
    void deactivate(bool unminimize);

Q_SIGNALS:
    void activeChanged();

private:
    bool match(const WId &wid, bool includehidden = true) const;
    QList<WId> m_minimizedWindows;
    bool m_active;
};

#endif //MINIMIZEALL_HEADER
