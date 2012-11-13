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

#ifndef ACTIVE_COMIC_MODEL_H
#define ACTIVE_COMIC_MODEL_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>

class ActiveComicModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        ComicKeyRole = Qt::UserRole+1,
        ComicTitleRole = Qt::UserRole+2,
        ComicIconRole = Qt::UserRole+3,
        ComicHighlightRole = Qt::UserRole+4
    };

    ActiveComicModel(QObject *parent = 0);

    void addComic(const QString &key, const QString &title, const QString &iconPath, bool highlight = true);

    int count() { return rowCount(QModelIndex()); }

    Q_INVOKABLE QVariantHash get(int i) const;

Q_SIGNALS:
    void countChanged();
};

#endif