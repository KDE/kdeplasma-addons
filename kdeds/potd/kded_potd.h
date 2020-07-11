/*
 *   Copyright 2020 Guo Yunhe <i@guoyunhe.me>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
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

#ifndef KDED_POTD_H
#define KDED_POTD_H

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>

#include <KConfig>
#include <KConfigGroup>
#include <KDEDModule>
#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>

class PotdModule: public KDEDModule
{
    Q_OBJECT

public:
    PotdModule(QObject* parent, const QList<QVariant>&);
    virtual ~PotdModule();

private Q_SLOTS:
    void fileChanged(const QString &path);

private:
    QString getSource();

    Plasma::DataEngineConsumer *consumer;
    Plasma::DataEngine *engine;
    QFileSystemWatcher *watcher;
    QString configPath;
    QString previousSource;
};

#endif
