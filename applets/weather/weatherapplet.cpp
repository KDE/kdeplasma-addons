/*
 *   SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2012 Luís Gabriel Lima <lampih@gmail.com>
 *   SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *   SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "weatherapplet.h"

#include <Plasma5Support/DataContainer>
#include <Plasma5Support/DataEngine>

// KF
#include <KConfigGroup>
#include <KLocalizedString>
#include <KUnitConversion/Unit>

using namespace KUnitConversion;

WeatherApplet::WeatherApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
    Plasma5Support::DataEngine *dataengine = dataEngine(QStringLiteral("weather"));
    const QVariantList plugins = dataengine->containerForSource(QLatin1String("ions"))->data().values();
    for (const QVariant &plugin : plugins) {
        const QStringList pluginInfo = plugin.toString().split(QLatin1Char('|'));
        if (pluginInfo.count() > 1) {
            m_providers[pluginInfo[1]] = pluginInfo[0];
        }
    }
    Q_EMIT providersChanged();
}

WeatherApplet::~WeatherApplet()
{
}

K_PLUGIN_CLASS(WeatherApplet)

#include "weatherapplet.moc"
