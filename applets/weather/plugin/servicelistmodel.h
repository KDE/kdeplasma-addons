/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef SERVICELISTMODEL_H
#define SERVICELISTMODEL_H

#include <Plasma/DataEngineConsumer>

#include <QAbstractListModel>
#include <QVector>


class ServiceItem
{
public:
    ServiceItem() {}
    ServiceItem(const QString& displayName, const QString& id)
    : displayName(displayName)
    , id(id)
    {}

    QString displayName;
    QString id;
    bool checked = false;
};

Q_DECLARE_METATYPE(ServiceItem)
Q_DECLARE_TYPEINFO(ServiceItem, Q_MOVABLE_TYPE);


class ServiceListModel : public QAbstractListModel, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(QStringList selectedServices MEMBER m_selectedServices WRITE setSelectedServices NOTIFY selectedServicesChanged)

public:
    explicit ServiceListModel(QObject* parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

public:
    void setSelectedServices(const QStringList& selectedServices);

Q_SIGNALS:
    void selectedServicesChanged();

private:
    QStringList m_selectedServices;
    QVector<ServiceItem> m_services;
};

#endif // SERVICELISTMODEL_H
