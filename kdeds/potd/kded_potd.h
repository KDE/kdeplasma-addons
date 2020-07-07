// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Guo Yunhe <i@guoyunhe.me>
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef KDED_POTD_H
#define KDED_POTD_H

#include <QObject>
#include <QString>

#include <KConfig>
#include <KConfigGroup>
#include <KDEDModule>
#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>
#include <KConfigWatcher>

class PotdModule: public KDEDModule
{
    Q_OBJECT

public:
    PotdModule(QObject* parent, const QList<QVariant>&);
    virtual ~PotdModule();

private Q_SLOTS:
    void configChanged();

private:
    QString getSource();

    Plasma::DataEngineConsumer *consumer = nullptr;
    Plasma::DataEngine *engine = nullptr;
    QString previousSource;

    KSharedConfig::Ptr config;
    KConfigWatcher::Ptr configWatcher;
};

#endif
