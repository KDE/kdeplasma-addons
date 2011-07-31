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

#include "FolderModel.h"
#include <KStandardDirs>
#include <KIcon>

#include <QFileInfo>

namespace Lancelot {
namespace Models {

class FolderModelPrivate {
public:
    FolderModelPrivate(FolderModel * parent);

    void addItem(const KUrl & url);
    void addItem(const KFileItem & item);

    void add(const KUrl & url);

    void deleteItem(const KFileItem & fileItem);
    void newItems(const KFileItemList & fileItems);

    int compare(const QString & item1, const QString & item2);

    KDirLister * dirLister;
    QString dirPath;
    QDir::SortFlags sort;

    class ItemInfo {
    public:
        /* url we already have QString url */
        uint time;
        bool isDir : 1;

    };

    QHash < QString, ItemInfo > items;

    class FolderModel * const q;
};

FolderModelPrivate::FolderModelPrivate(FolderModel * parent)
    : q(parent)
{
}

FolderModel::FolderModel(QString dirPath, QDir::SortFlags sort)
    : BaseModel(true), d(new FolderModelPrivate(this))
{
    d->dirPath = dirPath;
    d->sort = sort;

    if (!d->dirPath.endsWith(QDir::separator())) {
        d->dirPath += QDir::separator();
    }
    d->dirPath = d->dirPath.replace("//", "/");

    load();

    d->dirLister = new KDirLister();
    connect(d->dirLister, SIGNAL(clear()),
              this, SLOT(clear()));
    connect(d->dirLister, SIGNAL(deleteItem(KFileItem)),
              this, SLOT(deleteItem(KFileItem)));
    connect(d->dirLister, SIGNAL(newItems(KFileItemList)),
              this, SLOT(newItems(KFileItemList)));

    setSelfTitle(i18nc("Folder: path/to/folder", "Folder: %1", dirPath));

    d->dirLister->openUrl(KUrl(d->dirPath), KDirLister::Keep);
}

FolderModel::~FolderModel()
{
    disconnect(d->dirLister, 0, this, 0);
    delete d->dirLister;
    delete d;
}

void FolderModel::load()
{
    if (d->sort != QDir::NoSort) {
        return;
    }

    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup config = cfg.group("FolderModel");

    QStringList items = config.readEntry(d->dirPath, QStringList());
    foreach (const QString &item, items) {
        if (QFile::exists(item)) {
            d->addItem(KUrl(item));
        }
    }
}

void FolderModel::clear()
{
    BaseModel::clear();
}

void FolderModelPrivate::deleteItem(const KFileItem & fileItem)
{
    for (int i = 0; i < q->size(); i++) {
        FolderModel::Item item = q->itemAt(i);

        if (fileItem.localPath() == item.data.toString()
            || fileItem.url()    == item.data.toString()) {
            items.remove(item.data.toString());
            q->removeAt(i);
        }
    }
}

void FolderModelPrivate::newItems(const KFileItemList &items)
{
    foreach (const KFileItem &item, items) {
        QFileInfo info(item.localPath());
        if (info.isFile() || info.isDir()) {
            addItem(item);
        }
    }
}

int FolderModelPrivate::compare(const QString & item1, const QString & item2)
{
    if (!items.contains(item1) || !items.contains(item2)) {
        return QString::compare(item1, item2, Qt::CaseInsensitive);
    }

    /* default value - comparing strings or timestamps*/
    int result =
        sort & QDir::Time ?
                items[item2].time - items[item1].time :
        /* else */
                QString::compare(item1, item2, Qt::CaseInsensitive);

    /* sorting dirs first */
    if (sort & QDir::DirsFirst && items[item1].isDir != items[item2].isDir) {
        result =
            items[item1].isDir  ? -1 :
            items[item2].isDir  ?  1 :
            /* else */             result;
    }

    if (sort & QDir::Reversed) {
        result = - result;
    }

    return result;
}

void FolderModelPrivate::addItem(const KFileItem & item)
{
    QString file = item.url().url();

    if (items.contains(file)) {
        return;
    }

    items[file].time = item.time(KFileItem::ModificationTime).toTime_t();
    items[file].isDir = item.isDir();

    add(item.url());
}

void FolderModelPrivate::addItem(const KUrl & url)
{
    QString file = url.url();

    if (items.contains(file)) {
        return;
    }

    QFileInfo info(file);
    items[file].time = info.lastModified().toTime_t();
    items[file].isDir = info.isDir();

    add(url);
}

void FolderModelPrivate::add(const KUrl & url)
{
    for (int i = 0; i < q->size(); i++) {
        if (this->compare(q->itemAt(i).data.toString(), url.url()) > 0) {
            q->insertUrl(i, url);
            return;
        }
    }

   q->addUrl(url);
}

bool FolderModel::dataDropAvailable(int where, const QMimeData * mimeData)
{
    Q_UNUSED(where);
    return (mimeData->formats().contains("text/uri-list"));
}

void FolderModel::dataDropped(int where, const QMimeData * mimeData)
{
    if (mimeData->formats().contains("text/uri-list")) {
        int from = -1;

        KUrl url = KUrl(QString(mimeData->data("text/uri-list")));

        for (int i = 0; i < size(); i++) {
            if (url.path() == itemAt(i).data) {
                from = i;
                break;
            }
        }

        if (from != -1) {
            removeAt(from);
            insertUrl(where, url);
            save();
        }
    }
}

void FolderModel::save()
{
    QStringList items;
    for (int i = 0; i < size(); i++) {
        items << itemAt(i).data.toString();
    }

    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup config = cfg.group("FolderModel");

    config.writeEntry(d->dirPath, items);
    config.sync();
}

QString FolderModel::dirPath() const
{
    return d->dirPath;
}

void FolderModel::update()
{
    d->dirLister->updateDirectory(KUrl(d->dirPath));
}

} // namespace Models
} // namespace Lancelot

#include "FolderModel.moc"
