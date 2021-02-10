/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "servicelistmodel.h"

#include <Plasma/DataContainer>
#include <Plasma/DataEngine>

#include <KLocalizedString>

ServiceListModel::ServiceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    Plasma::DataEngine *dataengine = dataEngine(QStringLiteral("weather"));

    const QVariantList plugins = dataengine->containerForSource(QLatin1String("ions"))->data().values();
    for (const QVariant &plugin : plugins) {
        const QStringList pluginInfo = plugin.toString().split(QLatin1Char('|'));
        if (pluginInfo.count() > 1) {
            m_services.append(ServiceItem(pluginInfo[0], pluginInfo[1]));
        }
    }
}

int ServiceListModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_services.size();
    }

    return 0;
}

QHash<int, QByteArray> ServiceListModel::roleNames() const
{
    auto roleNames = QAbstractListModel::roleNames();
    roleNames.insert(Qt::CheckStateRole, "checked");
    return roleNames;
}

QVariant ServiceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_services.size()) {
        return QVariant();
    }

    const ServiceItem &item = m_services.at(index.row());

    switch (role) {
    case Qt::DisplayRole: {
        return i18nc("weather services provider name (id)", "%1 (%2)", item.displayName, item.id);
    case Qt::CheckStateRole:
        return item.checked;
    }
    }

    return QVariant();
}

bool ServiceListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || value.isNull()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        ServiceItem &item = m_services[index.row()];

        const bool checked = value.toBool();
        if (checked == item.checked) {
            return true;
        }

        item.checked = checked;
        Q_EMIT dataChanged(index, index);

        if (checked) {
            m_selectedServices.append(item.id);
        } else {
            m_selectedServices.removeAll(item.id);
        }

        Q_EMIT selectedServicesChanged();
        return true;
    }

    return false;
}

void ServiceListModel::setSelectedServices(const QStringList &selectedServices)
{
    if (m_selectedServices == selectedServices) {
        return;
    }

    m_selectedServices = selectedServices;

    for (int i = 0, size = m_services.size(); i < size; ++i) {
        ServiceItem &item = m_services[i];

        const bool checked = m_selectedServices.contains(item.id);
        if (checked == item.checked) {
            continue;
        }

        item.checked = checked;

        const QModelIndex index = createIndex(i, 0);
        Q_EMIT dataChanged(index, index);
    }

    Q_EMIT selectedServicesChanged();
}
