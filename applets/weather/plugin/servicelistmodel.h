/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SERVICELISTMODEL_H
#define SERVICELISTMODEL_H

#include <Plasma/DataEngineConsumer>

#include <QAbstractListModel>
#include <QVector>

class ServiceItem
{
public:
    ServiceItem()
    {
    }
    ServiceItem(const QString &displayName, const QString &id)
        : displayName(displayName)
        , id(id)
    {
    }

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
    explicit ServiceListModel(QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

public:
    void setSelectedServices(const QStringList &selectedServices);

Q_SIGNALS:
    void selectedServicesChanged();

private:
    QStringList m_selectedServices;
    QVector<ServiceItem> m_services;
};

#endif // SERVICELISTMODEL_H
