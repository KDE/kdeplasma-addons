// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Guo Yunhe <i@guoyunhe.me>

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

    Plasma::DataEngineConsumer *consumer = nullptr;
    Plasma::DataEngine *engine = nullptr;
    QFileSystemWatcher *watcher;
    QString configPath;
    QString previousSource;
};

#endif
