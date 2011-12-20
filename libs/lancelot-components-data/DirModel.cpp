/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "DirModel.h"
#include <KStandardDirs>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KDirLister>
#include <KDebug>
#include <KRun>

#include <QFileInfo>

class DirModelPrivate {
public:
    DirModelPrivate(DirModel * parent);

    class DirModel * const q;
    KDirModel * model;
};

DirModelPrivate::DirModelPrivate(DirModel * parent)
    : q(parent)
{
}

DirModel::DirModel()
    : d(new DirModelPrivate(this))
{
    d->model = new KDirModel(this);
    setSourceModel(d->model);
    setSortFoldersFirst(true);

    QHash <int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole]    = "display";
    roleNames[Qt::StatusTipRole]  = "description";
    roleNames[Qt::DecorationRole] = "decoration";
    setRoleNames(roleNames);

    for (int i = 0; i < d->model->columnCount(); i++) {
        kDebug() << "Header" << d->model->headerData(i, Qt::Horizontal);
    }
}

DirModel::~DirModel()
{
    delete d;
}

QString DirModel::dir() const
{
    return d->model->dirLister()->url().url();
}

void DirModel::setDir(const QString & dir)
{
    d->model->dirLister()->openUrl(KUrl(dir), KDirLister::Keep);
}

void DirModel::activate(int what)
{
    KUrl url = d->model->dirLister()->url().url();
    url.addPath(data(index(what, 0)).toString());
    kDebug() << url;
    new KRun(url, 0);
}

#include "DirModel.moc"
