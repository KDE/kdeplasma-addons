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
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[ComicKeyRole] = "key";
    newRoleNames[ComicTitleRole] = "title";
    newRoleNames[ComicIconRole] = "icon";
    newRoleNames[ComicHighlightRole] = "highlight";

    setRoleNames(newRoleNames);
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SIGNAL(countChanged()));
}

void ActiveComicModel::addComic(const QString &key, const QString &title, const QString &iconPath, bool highlight)
{
    QList<QStandardItem *> newRow;
    QStandardItem *item = new QStandardItem(title);

    item->setData(key, ComicKeyRole);
    item->setData(title, ComicTitleRole);
    item->setData(iconPath, ComicIconRole);
    item->setData(highlight, ComicHighlightRole);

    newRow << item;
    appendRow(newRow);
}

QVariantHash ActiveComicModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantHash hash;

    QHash<int, QByteArray>::const_iterator i;
    for (i = roleNames().constBegin(); i != roleNames().constEnd(); ++i) {
        hash[i.value()] = data(idx, i.key());
    }

    return hash;
}
