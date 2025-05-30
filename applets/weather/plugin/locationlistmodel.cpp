/*
 * SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 * SPDX-FileCopyrightText: 2016, 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2025 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "locationlistmodel.h"

#include <Plasma5Support/DataContainer>

#include <KLocalizedString>

#include <QCollator>
#include <QDebug>

WeatherValidator::WeatherValidator(Plasma5Support::DataEngine *weatherDataengine, const QString &ionName, QObject *parent)
    : QObject(parent)
    , m_weatherDataEngine(weatherDataengine)
    , m_ionName(ionName)
{
}

WeatherValidator::~WeatherValidator() = default;

void WeatherValidator::validate(const QString &location)
{
    const QString validationSource = m_ionName + QLatin1String("|validate|") + location;

    m_weatherDataEngine->connectSource(validationSource, this);
}

void WeatherValidator::dataUpdated(const QString &source, const Plasma5Support::DataEngine::Data &data)
{
    QMap<QString, QString> locationSources;

    m_weatherDataEngine->disconnectSource(source, this);

    const auto validationResult = data[QStringLiteral("validate")].toString().split(QLatin1Char('|'));

    if (validationResult.size() < 2) {
        Q_EMIT error(i18n("Cannot find '%1' using %2.", source, m_ionName));
    } else if (validationResult[1] == QLatin1String("valid") && validationResult.size() > 2) {
        const QString weatherSourcePrefix = validationResult[0] + QLatin1String("|weather|");
        int i = 3;

        const int lastFieldIndex = validationResult.size() - 1;
        while (i < lastFieldIndex) {
            if (validationResult[i] == QLatin1String("place")) {
                const QString &name = validationResult[i + 1];
                QString locationSource;
                if (i + 2 < lastFieldIndex && validationResult[i + 2] == QLatin1String("extra")) {
                    const QString &id = validationResult[i + 3];
                    locationSource = weatherSourcePrefix + name + QLatin1Char('|') + id;
                    i += 4;
                } else {
                    locationSource = weatherSourcePrefix + name;
                    i += 2;
                }
                locationSources.insert(name, locationSource);
            } else {
                ++i;
            }
        }
    } else if (validationResult[1] == QLatin1String("timeout")) {
        Q_EMIT error(i18n("Connection to %1 weather server timed out.", m_ionName));
    }

    Q_EMIT finished(locationSources);
}

LocationItem::LocationItem(const QString &source)
    : value(source)
{
    const QStringList sourceTerms = source.split(QLatin1Char('|'), Qt::SkipEmptyParts);
    if (sourceTerms.count() <= 2) {
        return;
    }
    weatherService = sourceTerms[0];
    weatherStation = sourceTerms[2];
    quality = relativeQuality(weatherService);
}

int LocationItem::relativeQuality(const QString &service) const
{
    static const QMap<QString, int> serviceQuality = {
        {"wettercom", -1}, // wetter.com does not provide current weather status
        {"dwd", 1}, // 7-day forecast and alerts, but some stations do not provide observation data
        {"bbcukmet", 2}, // 7-day forecast and observation. Worldwide general coverage
        {"noaa", 3}, // 12h 7-day forecast, observation and alerts
        {"envcan", 3}, // 12h 7-day forecast, observation and alerts
    };

    if (!serviceQuality.contains(service)) {
        return 1; // Fallback. Default quality
    }

    return serviceQuality.value(service);
}

LocationListModel::LocationListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_validatingInput(false)
    , m_checkedInCount(0)
{
    initProviders();
}

void LocationListModel::initProviders()
{
    qDeleteAll(m_validators);
    m_validators.clear();

    // The data engine is managed by the base class and should not be deleted
    Plasma5Support::DataEngine *weatherEngine = dataEngine(QStringLiteral("weather"));
    const QVariantList plugins = weatherEngine->containerForSource(QStringLiteral("ions"))->data().values();

    for (const QVariant &plugin : plugins) {
        const QStringList pluginInfo = plugin.toString().split(QLatin1Char('|'));
        if (pluginInfo.count() < 2) {
            continue;
        }

        const QString &ionName = pluginInfo[0];
        const QString &ionId = pluginInfo[1];

        m_serviceCodeToDisplayName[ionId] = ionName;

        auto validator = new WeatherValidator(weatherEngine, ionId, this);
        connect(validator, &WeatherValidator::error, this, &LocationListModel::validatorError);
        connect(validator, &WeatherValidator::finished, this, &LocationListModel::addSources);

        m_validators.append(validator);
    }
}

bool LocationListModel::hasProviders() const
{
    return !m_validators.isEmpty();
}

QVariant LocationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_locations.size()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return nameForListIndex(index.row());
    }

    return QVariant();
}

int LocationListModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_locations.size();
    }

    return 0;
}

bool LocationListModel::isValidatingInput() const
{
    return m_validatingInput;
}

QString LocationListModel::valueForListIndex(int listIndex) const
{
    if (0 <= listIndex && listIndex < m_locations.count()) {
        return m_locations.at(listIndex).value;
    }

    return QString();
}

QString LocationListModel::nameForListIndex(int listIndex) const
{
    if (0 <= listIndex && listIndex < m_locations.count()) {
        const LocationItem &item = m_locations.at(listIndex);
        if (!item.weatherService.isEmpty()) {
            return i18nc("A weather station location and the weather service it comes from",
                         "%1 (%2)",
                         item.weatherStation,
                         m_serviceCodeToDisplayName.value(item.weatherService, item.weatherService));
        }
    }

    return QString();
}

QString LocationListModel::providerName(const QString &providerId) const
{
    return m_serviceCodeToDisplayName.value(providerId, providerId);
}

void LocationListModel::searchLocations(const QString &searchString)
{
    m_checkedInCount = 0;

    m_searchString = searchString;

    if (!m_validatingInput) {
        m_validatingInput = true;
        Q_EMIT validatingInputChanged(true);
    }

    beginResetModel();
    m_locations.clear();
    endResetModel();

    if (searchString.isEmpty() || m_validators.isEmpty()) {
        completeSearch();
        return;
    }

    for (auto *validator : std::as_const(m_validators)) {
        validator->validate(m_searchString);
    }
}

void LocationListModel::validatorError(const QString &error)
{
    qDebug() << error;
}

void LocationListModel::addSources(const QMap<QString, QString> &sources)
{
    static QList<LocationItem> fallbackLocations;

    // TODO: be more elaborate and use beginInsertRows() & endInsertRows()
    beginResetModel();

    for (const auto &source : sources) {
        const auto item = LocationItem(source);
        if (item.weatherStation.isEmpty()) {
            continue;
        }
        if (item.quality >= 0) {
            m_locations << item;
        } else {
            fallbackLocations << item;
        }
    }

    QCollator collator;
    std::ranges::stable_sort(m_locations, [&collator](const auto &a, const auto &b) {
        // Promote services with better quality to the top of the list
        if (a.quality != b.quality) {
            return a.quality > b.quality;
        }
        // Otherwise sort alphabetically to produce a stable list
        return collator.compare(a.weatherStation, b.weatherStation) <= 0;
    });

    endResetModel();

    ++m_checkedInCount;
    if (m_checkedInCount >= m_validators.count()) {
        if (m_locations.isEmpty()) {
            beginResetModel();
            m_locations = fallbackLocations;
            endResetModel();
        }
        fallbackLocations.clear();
        completeSearch();
    }
}

void LocationListModel::completeSearch()
{
    m_validatingInput = false;
    const bool success = !m_locations.empty();
    Q_EMIT locationSearchDone(success, m_searchString);
    Q_EMIT validatingInputChanged(false);
}

void LocationListModel::clear()
{
    beginResetModel();
    m_locations.clear();
    endResetModel();

    m_checkedInCount = 0;
    m_validatingInput = false;
    Q_EMIT validatingInputChanged(false);
}
