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

namespace Plasma
{
    class DeclarativeWidget;
}

class WeatherApplet : public WeatherPopupApplet
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap panelModel READ panelModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList fiveDaysModel READ fiveDaysModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList detailsModel READ detailsModel NOTIFY modelUpdated)
    Q_PROPERTY(QVariantList noticesModel READ noticesModel NOTIFY modelUpdated)
public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet();

    void init();
    QGraphicsWidget *graphicsWidget();
    QVariantMap panelModel() const { return m_panelModel; }
    QVariantList fiveDaysModel() const { return m_fiveDaysModel; }
    QVariantList detailsModel() const { return m_detailsModel; }
    QVariantList noticesModel() const { return m_noticesModel; }

signals:
    void modelUpdated();

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void invokeBrowser(const QString &url = QString()) const;

protected Q_SLOTS:
    void configAccepted();
    void toolTipAboutToShow();

protected:
    void constraintsEvent(Plasma::Constraints constraints);

private:
    bool isValidData(const QVariant &data) const;
    void resetPanelModel();
    void updatePanelModel(const Plasma::DataEngine::Data &data);
    void updateFiveDaysModel(const Plasma::DataEngine::Data &data);
    void updateDetailsModel(const Plasma::DataEngine::Data &data);
    void updateNoticesModel(const Plasma::DataEngine::Data &data);
    QString convertTemperature(KUnitConversion::UnitPtr format, QString value,
                               int type, bool rounded = false, bool degreesOnly = false);

    Plasma::DeclarativeWidget *m_declarativeWidget;

    QString m_creditUrl;

    QVariantMap m_panelModel;
    QVariantList m_fiveDaysModel;
    QVariantList m_detailsModel;
    QVariantList m_noticesModel;
};

K_EXPORT_PLASMA_APPLET(weather, WeatherApplet)

#endif
