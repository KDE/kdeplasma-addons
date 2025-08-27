/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "forecastdata.h"
#include "locationsdata.h"

#include <KPluginMetaData>

class IonControl;

/*!
 * \class WeatherDataMonitor
 *
 * \brief The WeatherDataMonitor class provides a way for receiving and updating forecast data.
 *
 * The WeatherDataMonitor class is a singleton that manages weather data providers. It loads providers
 * as a Qt plugins, use them to retrieve locations and forecasts (based on the place info of the locations),
 * and monitors the lifecycle of the providers.
 */
class WeatherDataMonitor : public QObject
{
    Q_OBJECT

public:
    /*!
     * Return the instance of WeatherDataMonitor.
     */
    static std::shared_ptr<WeatherDataMonitor> instance();

    /*!
     * Return the names of available providers.
     */
    QStringList getProviders() const;

    /*!
     * Return the quality of provider as \c int (the higher the number the better).
     *
     * \a providerName The name of a provider.
     */
    int providerQuality(const QString &providerName) const;

    /*!
     * Return the display name of a provider.
     *
     * \a providerName The name of a provider.
     */
    QString providerDisplayName(const QString &providerName) const;

    /*!
     * Return the \c LocationData associated with given provider. If provider with
     * such name does not exist return  \c nullptr.
     *
     * \a providerName The name of a provider.
     */
    std::shared_ptr<LocationsData> getLocationData(const QString &providerName);

    /*!
     * updates \c LocationsData with new locations according to \c searchString
     *
     * \a locationData \c std::shared_ptr which points to \c LocationsData which needs to be updated.
     * \a searchString \c QString name by which locations will be searched.
     */
    void updateLocationData(const std::shared_ptr<LocationsData> &locationData, const QString &searchString);

    /*!
     * Return the \c std::shared_ptr to \c ForecastData associated with given provider and place info.
     * If provider with such name does not exist return shared pointer with \c nullptr.
     *
     * \a providerName The name of a provider.
     * \a placeInfo A Specific \c QString which is used by provider to retrieve forecast. Сan be obtained from LocationsData.
     */
    std::shared_ptr<ForecastData> getForecastData(const QString &providerName, const QString &placeInfo);

    /*!
     * updates the \c ForecastData with a new forecast
     *
     * \a forecastData The \c std::shared_ptr which points to \c ForecastData which needs to be updated.
     */
    void updateForecastData(const std::shared_ptr<ForecastData> &forecastData);

    ~WeatherDataMonitor() override;

private:
    explicit WeatherDataMonitor(QObject *parent = nullptr);

    /*!
     * Used whenever \c LocationsData or \c ForecastData is removed to clear unneeded data and unload plugin
     */
    void finishRemoving(const QString &providerName);

    /*!
     * Used by \c getLocationData or \c getForecastData, if needed, to create IonInfo and load plugin
     */
    bool createIonControl(const QString &providerName);

public Q_SLOTS:
    /*!
     * Called whenever \c getForecastData is destroyed to manage the usages of the plugin
     */
    void onWeatherDataRemoved(const QString &providerName, const QString &place);

    /*!
     * Called whenever \c LocationsData is destroyed to manage the usages of the plugin
     */
    void onLocationDataRemoved(const QString &provider);

private:
    /*!
     * \class IonInfo
     *
     * Internal struct used to monitor the usages of a plugin.
     *
     */
    struct IonInfo {
        std::shared_ptr<IonControl> ionControl;
        int locationCount = 0;
        QList<std::pair<QString, std::weak_ptr<ForecastData>>> forecastDataList;
    };

    QHash<QString, IonInfo> m_ionControls;
    QHash<QString, KPluginMetaData> m_ions;

    QStringList m_providers;
};
