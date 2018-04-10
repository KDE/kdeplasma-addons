/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
 *   Copyright (C) 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef WEATHERAPPLET_H
#define WEATHERAPPLET_H

#include <KUnitConversion/Unit>
#include <KUnitConversion/Converter>

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>
#include <Plasma/Applet>

class KNotification;
class QTimer;

class WeatherApplet : public Plasma::Applet, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap panelModel READ panelModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList forecastModel READ forecastModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList detailsModel READ detailsModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList noticesModel READ noticesModel NOTIFY modelUpdated)

    Q_PROPERTY(QVariantMap configuration READ configuration NOTIFY configurationChanged FINAL)
    // used for making this information available to the config pages
    Q_PROPERTY(bool needsToBeSquare MEMBER m_needsToBeSquare NOTIFY needsToBeSquareChanged FINAL)

public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet() override;

public: // Plasma::Applet API
    void init() override;
    void configChanged() override;

public: // QML config control API
    /**
     * @return currently used config values
     */
    Q_INVOKABLE QVariantMap configValues() const;

    /**
     * @param configChanges config key-value entries which have changed
     */
    Q_INVOKABLE void saveConfig(const QVariantMap& configChanges);

public:
    QVariantMap panelModel() const { return m_panelModel; }
    QVariantList forecastModel() const { return m_forecastModel; }
    QVariantList detailsModel() const { return m_detailsModel; }
    QVariantList noticesModel() const { return m_noticesModel; }
    QVariantMap configuration() const { return m_configuration; }

public Q_SLOTS: // callback for the weather dataengine
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void modelUpdated();
    void configurationChanged();
    void needsToBeSquareChanged();

private:
    void connectToEngine();
    void giveUpBeingBusy();

    bool isValidData(const QVariant &data) const;
    bool isValidData(const QString &data) const;
    void resetPanelModel();
    void updatePanelModel(const Plasma::DataEngine::Data &data);
    void updateForecastModel(const Plasma::DataEngine::Data &data);
    void updateDetailsModel(const Plasma::DataEngine::Data &data);
    void updateNoticesModel(const Plasma::DataEngine::Data &data);
    QString convertTemperature(const KUnitConversion::Unit& format, float value,
                               int type, bool rounded = false, bool degreesOnly = false);
    QString convertTemperature(const KUnitConversion::Unit& format, const QVariant& value,
                               int type, bool rounded = false, bool degreesOnly = false);
    KUnitConversion::Unit unit(const QString& unit);

    void onTimeoutNotificationClosed();

private:
    KUnitConversion::Converter m_converter;
    KUnitConversion::Unit m_displayTemperatureUnit;
    KUnitConversion::Unit m_displaySpeedUnit;
    KUnitConversion::Unit m_displayPressureUnit;
    KUnitConversion::Unit m_displayVisibilityUnit;

    int m_updateInterval = 0; // in minutes
    QString m_source;

    QTimer* m_busyTimer = nullptr;
    KNotification* m_timeoutNotification = nullptr;
    QMetaObject::Connection m_timeoutNotificationConnection;

    QVariantMap m_panelModel;
    QVariantList m_forecastModel;
    QVariantList m_detailsModel;
    QVariantList m_noticesModel;

    QVariantMap m_configuration;
    bool m_needsToBeSquare = false;
};

#endif
