/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
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

#include <plasmaweather/weatherpopupapplet.h>

class WeatherApplet : public WeatherPopupApplet
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap panelModel READ panelModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList fiveDaysModel READ fiveDaysModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList detailsModel READ detailsModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList noticesModel READ noticesModel NOTIFY modelUpdated)

public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet() override;

public: // Plasma::Applet API
    void init() override;

public:
    QVariantMap panelModel() const { return m_panelModel; }
    QVariantList fiveDaysModel() const { return m_fiveDaysModel; }
    QVariantList detailsModel() const { return m_detailsModel; }
    QVariantList noticesModel() const { return m_noticesModel; }

Q_SIGNALS:
    void modelUpdated();

public: // WeatherPopupApplet API
    void saveConfig(const QVariantMap& configChanges) override;
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data) override;

private:
    bool isValidData(const QVariant &data) const;
    bool isValidData(const QString &data) const;
    void resetPanelModel();
    void updatePanelModel(const Plasma::DataEngine::Data &data);
    void updateFiveDaysModel(const Plasma::DataEngine::Data &data);
    void updateDetailsModel(const Plasma::DataEngine::Data &data);
    void updateNoticesModel(const Plasma::DataEngine::Data &data);
    QString convertTemperature(const KUnitConversion::Unit& format, float value,
                               int type, bool rounded = false, bool degreesOnly = false);
    QString convertTemperature(const KUnitConversion::Unit& format, const QVariant& value,
                               int type, bool rounded = false, bool degreesOnly = false);

private:
    QVariantMap m_panelModel;
    QVariantList m_fiveDaysModel;
    QVariantList m_detailsModel;
    QVariantList m_noticesModel;
};

#endif
