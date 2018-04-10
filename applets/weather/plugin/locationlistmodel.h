/*
 * Copyright 2016  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOCATIONLISTMODEL_H
#define LOCATIONLISTMODEL_H

#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>

#include <QAbstractListModel>
#include <QVector>
#include <QMap>

class WeatherValidator : public QObject
{
    Q_OBJECT
public:
    WeatherValidator(Plasma::DataEngine* weatherDataengine, const QString& ionName, QObject* parent = nullptr);
    ~WeatherValidator() override;

    /**
     * @param location the name of the location to find
     */
    void validate(const QString& location);

Q_SIGNALS:
    /**
     * Emitted when an error in validation occurs
     **/
    void error(const QString& message);

    /**
     * Emitted when validation is done
     * @param sources a mapping of user-friendly names to the DataEngine source
     **/
    void finished(const QMap<QString, QString>& sources);

public Q_SLOTS: // callback for the weather dataengine
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

private:
    Plasma::DataEngine* m_weatherDataEngine;
    QString m_ionName;
};


class LocationItem
{
public:
    LocationItem() {}
    LocationItem(const QString &_weatherStation, const QString &_weatherService, const QString &_value)
    : weatherStation(_weatherStation)
    , weatherService(_weatherService)
    , value(_value)
    {}

    QString weatherStation;
    QString weatherService;
    QString value;
};

Q_DECLARE_METATYPE(LocationItem)
Q_DECLARE_TYPEINFO(LocationItem, Q_MOVABLE_TYPE);


class LocationListModel : public QAbstractListModel, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(bool validatingInput READ isValidatingInput NOTIFY validatingInputChanged)

public:
    explicit LocationListModel(QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

public:
    bool isValidatingInput() const;

public:
    Q_INVOKABLE QString nameForListIndex(int listIndex) const;
    Q_INVOKABLE QString valueForListIndex(int listIndex) const;
    Q_INVOKABLE void searchLocations(const QString &searchString, const QStringList& services);

Q_SIGNALS:
    void validatingInputChanged(bool validatingInput);
    void locationSearchDone(bool success, const QString &searchString);

private:
    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);
    void completeSearch();

private:
    QVector<LocationItem> m_locations;

    bool m_validatingInput;
    QString m_searchString;
    int m_checkedInCount;
    QVector<WeatherValidator*> m_validators;
};

#endif // LOCATIONLISTMODEL_H
