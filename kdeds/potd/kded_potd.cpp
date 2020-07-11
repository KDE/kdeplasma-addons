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

#include "kded_potd.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PotdModule, "kded_potd.json")

PotdModule::PotdModule(QObject* parent, const QList<QVariant>&): KDEDModule(parent)
{
    consumer = new Plasma::DataEngineConsumer();
    engine = consumer->dataEngine(QStringLiteral("potd"));
    configPath = QStandardPaths::locate(QStandardPaths::ConfigLocation,  QStringLiteral("kscreenlockerrc"));

    previousSource = getSource();
    engine->connectSource(previousSource, this); // trigger caching, no need to handle data

    watcher = new QFileSystemWatcher(this);
    watcher->addPath(configPath);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &PotdModule::fileChanged);
}

PotdModule::~PotdModule()
{
    delete consumer;
}

void PotdModule::fileChanged(const QString &path)
{
    Q_UNUSED(path);
    engine->disconnectSource(previousSource, this);
    previousSource = getSource();
    engine->connectSource(previousSource, this);

    // For some reason, Qt *rc files are always recreated instead of modified.
    // Recreated files were removed from watchers and have to be added again.
    watcher->addPath(configPath);
}

QString PotdModule::getSource()
{
    KConfig config(configPath);

    KConfigGroup greeterGroup = config.group(QStringLiteral("Greeter"));
    QString plugin = greeterGroup.readEntry(QStringLiteral("WallpaperPlugin"), QString());
    if (plugin != QStringLiteral("org.kde.potd")) {
        return QStringLiteral("");
    }

    KConfigGroup potdGroup = greeterGroup
                               .group(QStringLiteral("Wallpaper"))
                               .group(QStringLiteral("org.kde.potd"))
                               .group(QStringLiteral("General"));
    QString provider = potdGroup.readEntry(QStringLiteral("Provider"), QString());
    if (provider == QStringLiteral("unsplash")) {
        QString category = potdGroup.readEntry(QStringLiteral("Category"), QString());
        return provider + QStringLiteral(":") + category;
    } else {
        return provider;
    }
}

#include "kded_potd.moc"
