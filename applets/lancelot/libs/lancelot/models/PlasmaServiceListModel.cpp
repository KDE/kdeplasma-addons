/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PlasmaServiceListModel.h"
#include <Plasma/Service>
#include <Plasma/DataEngineManager>
#include <KIcon>
#include <QDebug>
#include <KConfigGroup>

namespace Lancelot {

class PlasmaServiceListModel::Private {
public:
    Private()
        : engine(NULL)
    {}

    Plasma::DataEngine * engine;
    Plasma::DataEngine::Data data;

    QString title;
    QIcon icon;
};

PlasmaServiceListModel::PlasmaServiceListModel(QString dataEngine)
    : d(new Private())
{
    d->engine = Plasma::DataEngineManager::self()->loadEngine(dataEngine);

    if (!d->engine->sources().contains(".metadata")) {
        qDebug() << "PlasmaServiceListModel:" << dataEngine << "is not a lancelot model - it doesn't have the .metadata structure";
        d->engine = NULL;
        return;
    }

    Plasma::DataEngine::Data data =
        d->engine->query(".metadata");
    if (!data.contains("lancelot") ||
        data["lancelot"].toMap()["version"] != "1.0"
    ) {
        qDebug() << "PlasmaServiceListModel:" << dataEngine << "is not a lancelot model - the version is not valid";
        d->engine = NULL;
        return;
    }
    qDebug() << "PlasmaServiceListModel:" << data
             << data["lancelot"].toMap()["modelTitle"].toString()
             << data["lancelot"].toMap()["modelIcon"].toString();

    d->icon = KIcon(data["lancelot"].toMap()["modelIcon"].toString());

    d->title = data["lancelot"].toMap()["modelTitle"].toString();
    d->icon = KIcon(data["lancelot"].toMap()["modelIcon"].toString());

    d->engine->connectSource("data", this);
}

PlasmaServiceListModel::~PlasmaServiceListModel()
{
    delete d;
}

QString PlasmaServiceListModel::title(int index) const
{
    if (index < 0 || index >= size()) {
        return QString();
    }

    QStringList list = d->data["title"].toStringList();
    return list.at(index);
}

QString PlasmaServiceListModel::description(int index) const
{
    if (index < 0 || index >= size()) {
        return QString();
    }

    QStringList list = d->data["description"].toStringList();
    return list.at(index);
}

QIcon PlasmaServiceListModel::icon(int index) const
{
    if (index < 0 || index >= size()) {
        return QIcon();
    }

    QStringList list = d->data["icon"].toStringList();
    return KIcon(list.at(index));
}

bool PlasmaServiceListModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

int PlasmaServiceListModel::size() const
{
    return d->data["title"].toStringList().size();
}

void PlasmaServiceListModel::dataUpdated(const QString & name,
        const Plasma::DataEngine::Data & data)
{
    if (name == "data") {
        d->data = data;
        emit updated();
    }
}

QString PlasmaServiceListModel::selfTitle() const
{
    return d->title;
}

QIcon PlasmaServiceListModel::selfIcon() const
{
    return d->icon;
}

void PlasmaServiceListModel::activate(int index)
{
    Plasma::Service * service =
        d->engine->serviceForSource("data");
    KConfigGroup cg = service->operationDescription("activate");

    QStringList list = d->data["data"].toStringList();
    cg.writeEntry("data", list.at(index));

    service->startOperationCall(cg);
}

} // namespace Lancelot

