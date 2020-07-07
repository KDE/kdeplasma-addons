// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Guo Yunhe <i@guoyunhe.me>
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "kded_potd.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PotdModule, "kded_potd.json")

PotdModule::PotdModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , config(KSharedConfig::openConfig(QStringLiteral("kscreenlockerrc"), KConfig::CascadeConfig))
    , configWatcher(KConfigWatcher::create(config))
{
    connect(configWatcher.data(), &KConfigWatcher::configChanged,
            this, &PotdModule::configChanged);
    configChanged();
}

PotdModule::~PotdModule()
{
    delete consumer;
}

void PotdModule::configChanged()
{
    if (engine) {
        engine->disconnectSource(previousSource, this);
    }

    previousSource = getSource();
    if (!previousSource.isEmpty()) {
        if (!consumer) {
            consumer = new Plasma::DataEngineConsumer;
            engine = consumer->dataEngine(QStringLiteral("potd"));
            engine->connectSource(previousSource, this); // trigger caching, no need to handle data
        }

        engine->connectSource(previousSource, this);
    }
}

QString PotdModule::getSource()
{
    KConfigGroup greeterGroup = config->group(QStringLiteral("Greeter"));
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
