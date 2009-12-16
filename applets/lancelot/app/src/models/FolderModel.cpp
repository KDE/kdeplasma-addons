/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "FolderModel.h"
#include <KStandardDirs>
#include <KIcon>

namespace Models {

FolderModel::FolderModel(QString dirPath, QDir::SortFlags sort)
    : BaseModel(true), m_dirPath(dirPath), m_sort(sort)
{
    if (!m_dirPath.endsWith(QDir::separator())) {
        m_dirPath += QDir::separator();
    }
    m_dirPath = m_dirPath.replace("//", "/");

    load();

    m_dirLister = new KDirLister();
    connect(m_dirLister, SIGNAL(clear()),
              this, SLOT(clear()));
    connect(m_dirLister, SIGNAL(deleteItem(const KFileItem &)),
              this, SLOT(deleteItem(const KFileItem &)));
    connect(m_dirLister, SIGNAL(newItems(const KFileItemList &)),
              this, SLOT(newItems(const KFileItemList &)));

    m_dirLister->openUrl(KUrl(m_dirPath), KDirLister::Keep);
}

FolderModel::~FolderModel()
{
    delete m_dirLister;
}

void FolderModel::load()
{
    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup config = cfg.group("FolderModel");

    QStringList items = config.readEntry(m_dirPath, QStringList());
    foreach (const QString &item, items) {
        if (QFile::exists(item)) {
            addItem(QUrl(item).toString());
        }
    }
}

void FolderModel::clear()
{
    BaseModel::clear();
}

void FolderModel::deleteItem(const KFileItem & fileItem)
{
    for (int i = 0; i < size(); i++) {
        Item item = itemAt(i);

        if (fileItem.localPath() == item.data.toString()) {
            m_items.removeAll(item.data.toString());
            removeAt(i);
        }
    }
}

void FolderModel::newItems(const KFileItemList &items)
{
    foreach (const KFileItem &item, items) {
        QFileInfo info(item.localPath());
        if (info.isFile() || info.isDir()) {
            addItem(item.localPath());
        }
    }
}

void FolderModel::addItem(const QString & url)
{
    if (m_items.contains(url)) {
        return;
    }

    qDebug() << "FolderModel::addItem:" << url;
    m_items << url;
    addUrl(url);
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

    qDebug() << "FolderModel::save:"
        << m_dirPath
        << items;

    KConfig cfg(KStandardDirs::locate("config", "lancelotrc"));
    KConfigGroup config = cfg.group("FolderModel");

    config.writeEntry(m_dirPath, items);
    config.sync();
}

} // namespace Models

#include "FolderModel.moc"
