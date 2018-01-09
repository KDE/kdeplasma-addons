/*
 * Copyright 2009  Petri Damstén <damu@iki.fi>
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

#include "locationlistmodel.h"

#include <plasmaweather/weathervalidator.h>

#include <Plasma/DataContainer>
#include <Plasma/DataEngine>

#include <KLocalizedString>

#include <QDebug>

LocationListModel::LocationListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_validatingInput(false)
    , m_checkedInCount(0)
{
}

QVariant LocationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_locations.size()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole: return nameForListIndex(index.row());
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
        const LocationItem& item = m_locations.at(listIndex);
        if (!item.weatherService.isEmpty()) {
            return i18nc("A weather station location and the weather service it comes from",
                        "%1 (%2)", item.weatherStation, item.weatherService);
        }
    }

    return QString();
}

void LocationListModel::searchLocations(const QString &searchString, const QStringList& services)
{
    m_checkedInCount = 0;

    // reset current validators
    qDeleteAll(m_validators);
    m_validators.clear();

    m_searchString = searchString;

    if (!m_validatingInput) {
        m_validatingInput = true;
        emit validatingInputChanged(true);
    }

    beginResetModel();
    m_locations.clear();
    endResetModel();

    if (searchString.isEmpty()) {
        completeSearch();
        return;
    }

    Plasma::DataEngine* dataengine = dataEngine(QStringLiteral("weather"));

    const QVariantList plugins = dataengine->containerForSource(QStringLiteral("ions"))->data().values();
    for (const QVariant& plugin : plugins) {
        const QStringList pluginInfo = plugin.toString().split(QLatin1Char('|'));
        if (pluginInfo.count() > 1) {
            const QString& ionId = pluginInfo[1];
            if (!services.contains(ionId)) {
                continue;
            }
            //qDebug() << "ion: " << pluginInfo[0] << pluginInfo[1];
            //d->ions.insert(pluginInfo[1], pluginInfo[0]);

            WeatherValidator *validator = new WeatherValidator(this);
            connect(validator, &WeatherValidator::error, this, &LocationListModel::validatorError);
            connect(validator, &WeatherValidator::finished, this, &LocationListModel::addSources);
            validator->setDataEngine(dataengine);
            validator->setIon(ionId);

            m_validators.append(validator);
        }
    }

    for (WeatherValidator* validator : qAsConst(m_validators)) {
        validator->validate(m_searchString, true);
    }
}

void LocationListModel::validatorError(const QString &error)
{
    qDebug() << error;
}

void LocationListModel::addSources(const QMap<QString, QString> &sources)
{
    QMapIterator<QString, QString> it(sources);

    // TODO: be more elaborate and use beginInsertRows() & endInsertRows()
    beginResetModel();

    while (it.hasNext()) {
        it.next();
        const QStringList list = it.value().split(QLatin1Char('|'), QString::SkipEmptyParts);
        if (list.count() > 2) {
            qDebug() << list;
            m_locations.append(LocationItem(list[2], list[0], it.value()));
        }
    }

    endResetModel();

    ++m_checkedInCount;
    if (m_checkedInCount >= m_validators.count()) {
        completeSearch();
    }
}

void LocationListModel::completeSearch()
{
    m_validatingInput = false;
    const bool success = !m_locations.empty();
    emit locationSearchDone(success, m_searchString);
    emit validatingInputChanged(false);
}
