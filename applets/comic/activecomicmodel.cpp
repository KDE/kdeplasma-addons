/*
 * Copyright 2012  Reza Fatahilah Shah <rshah0385@kireihana.com>
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

#include "activecomicmodel.h"

ActiveComicModel::ActiveComicModel(QObject *parent)
    : QStandardItemModel(0, 1, parent)
{
    connect(this, &ActiveComicModel::modelReset,
            this, &ActiveComicModel::countChanged);
    connect(this, &ActiveComicModel::rowsInserted,
            this, &ActiveComicModel::countChanged);
    connect(this, &ActiveComicModel::rowsRemoved,
            this, &ActiveComicModel::countChanged);
}

QHash<int, QByteArray> ActiveComicModel::roleNames() const
{
    auto roleNames = QStandardItemModel::roleNames();
    roleNames.insert(ComicKeyRole, "key");
    roleNames.insert(ComicTitleRole, "title");
    roleNames.insert(ComicIconRole, "icon");
    roleNames.insert(ComicHighlightRole, "highlight");

    return roleNames;
}

void ActiveComicModel::addComic(const QString& key, const QString& title, const QIcon& icon, bool highlight)
{
    QList<QStandardItem *> newRow;
    QStandardItem *item = new QStandardItem(title);

    item->setData(key, ComicKeyRole);
    item->setData(title, ComicTitleRole);
    item->setData(icon, ComicIconRole);
    item->setData(highlight, ComicHighlightRole);

    newRow << item;
    appendRow(newRow);
}

QVariantHash ActiveComicModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantHash hash;

    const auto roleNames = this->roleNames();
    hash.reserve(roleNames.size());
    for (auto end = roleNames.constEnd(), it = roleNames.constBegin(); it != end; ++it) {
        hash.insert(QString::fromUtf8(it.value()), data(idx, it.key()));
    }

    return hash;
}
