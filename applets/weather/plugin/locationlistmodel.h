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

#include <Plasma/DataEngine>

#include <QAbstractListModel>
#include <QVector>
#include <QMap>

class WeatherValidator;


class LocationItem
{
public:
    LocationItem() {}
    explicit LocationItem(const QString &_name, const QString &_value) : name(_name), value(_value) {}

    QString name;
    QString value;
};

Q_DECLARE_METATYPE(LocationItem)
Q_DECLARE_TYPEINFO(LocationItem, Q_MOVABLE_TYPE);


class LocationListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool validatingInput READ isValidatingInput NOTIFY validatingInputChanged)
    Q_PROPERTY(Plasma::DataEngine* dataEngine READ dataEngine WRITE setDataEngine NOTIFY dataEngineChanged)

public:
    explicit LocationListModel(QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

public:
    bool isValidatingInput() const;
    Plasma::DataEngine* dataEngine() const;
    void setDataEngine(Plasma::DataEngine* dataengine);

public:
    Q_INVOKABLE QString valueForListIndex(int listIndex) const;
    Q_INVOKABLE void searchLocations(const QString &searchString);

Q_SIGNALS:
    void validatingInputChanged(bool validatingInput);
    void dataEngineChanged(Plasma::DataEngine* dataEngine);
    void locationSearchDone(bool success, const QString &searchString);

private:
    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);

private:
    QVector<LocationItem> m_locations;

    Plasma::DataEngine* m_dataengine;
    bool m_validatingInput;
    QString m_searchString;
    int m_checkedInCount;
    QVector<WeatherValidator*> m_validators;
};

#endif // LOCATIONLISTMODEL_H
