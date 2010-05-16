/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "AvailableModels.h"
#include <KStandardDirs>
#include <KFileDialog>
#include <KDirSelectDialog>
#include <KIcon>
#include <KDebug>

#include <Lancelot/Models/Serializator>

namespace Lancelot {
namespace Models {

AvailableModels * AvailableModels::instance = NULL;

AvailableModels * AvailableModels::self()
{
    if (!instance) {
        instance = new AvailableModels();
    }
    return instance;
}

AvailableModels::AvailableModels()
    : StandardActionTreeModel()
{
    StandardActionTreeModel::Item * item = root();

    // Applications model
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Favorite applications"), QString::null,
           KIcon("favorites"),
           "FavoriteApplications");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Applications category..."), QString::null,
           KIcon("plasmaapplet-shelf"),
           "Applications");

    // Places
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Places"), QString::null,
           KIcon("folder"),
           "Places");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Folder..."), QString::null,
           KIcon("folder"),
           "Folder");

    // Devices
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Removable devices"), QString::null,
           KIcon("media-optical"),
           "Devices/Removable");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Fixed devices"), QString::null,
           KIcon("drive-harddisk"),
           "Devices/Fixed");

    // System applications
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("System tools"), QString::null,
           KIcon("computer"),
           "System");

    // Documents
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("New documents"), QString::null,
           KIcon("document-new"),
           "NewDocuments");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Open documents"), QString::null,
           KIcon("document-edit"),
           "OpenDocuments");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Recent documents"), QString::null,
           KIcon("document-open-recent"),
           "RecentDocuemnts");

    // Contacts
    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Online contacts"), QString::null,
           KIcon("kopete"),
           "Contacts");

    item->children <<
        new StandardActionTreeModel::Item(
           i18n("Unread messages"), QString::null,
           KIcon("kmail"),
           "Messages");


}

AvailableModels::~AvailableModels()
{
}

StandardActionTreeModel * AvailableModels::createChild(int index)
{
    Q_UNUSED(index);

    return NULL;
}

ActionListModel * AvailableModels::modelForSerializedData(const QString & serializedData)
{
    QMap < QString, QString > data = Lancelot::Models::Serializator::deserialize(serializedData);

    return 0;
}

QString AvailableModels::serializedDataForItem(int index)
{
    QMap < QString, QString > data;
    data["version"] = "1.0";
    data["type"] = "list";
    data["model"] = itemAt(index)->data.toString();

    if (data["model"] == "Folder") {
        QString dir = KFileDialog::getExistingDirectory();
        data["model"] = "Folder " + dir;
    } else if (data["model"] == "Applications") {
        data["model"] = "Folder " +
            KDirSelectDialog::selectDirectory(KUrl("applications:/")).url();
    }

    if (data["model"] == "Folder ") {
        return QString::null;
    }

    return Serializator::serialize(data);
}

ActionListModel * AvailableModels::modelForItem(int index)
{
    return modelForSerializedData(
        serializedDataForItem(index));
}

QString AvailableModels::titleForModel(const QString & modelId)
{
    kDebug() << modelId;

    if (modelId.startsWith("Folder ")) {
        QString result = modelId;
        result.replace("Folder ", QString());
        return result;
    }

    for (int i = 0; i < size(); i++) {
        if (itemAt(i)->data.toString() == modelId) {
            return itemAt(i)->title;
        }
    }

    return QString();
}

} // namespace Models
} // namespace Lancelot

#include "AvailableModels.moc"
