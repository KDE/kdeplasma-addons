/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef QUICKLAUNCH_P_H
#define QUICKLAUNCH_P_H

#include <QUrl>
#include <QObject>
#include <QVariantMap>

class QuicklaunchPrivate : public QObject
{
    Q_OBJECT

public:
    explicit QuicklaunchPrivate(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap launcherData(const QUrl &url);
    Q_INVOKABLE void openUrl(const QUrl &url);
    Q_INVOKABLE void openExec(const QString &exec);

    Q_INVOKABLE void addLauncher(bool isPopup = false);
    Q_INVOKABLE void editLauncher(QUrl url, int index, bool isPopup = false);

Q_SIGNALS:
    void launcherAdded(const QString &url, bool isPopup);
    void launcherEdited(const QString &url, int index, bool isPopup);
};

#endif // QUICKLAUNCH_P_H
