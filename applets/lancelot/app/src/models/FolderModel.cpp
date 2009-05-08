/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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
#include <KIcon>

namespace Models {

FolderModel::FolderModel(QString dirPath, QDir::SortFlags sort)
    : BaseModel(true), m_dirPath(dirPath), m_sort(sort)
{
    if (!m_dirPath.endsWith(QDir::separator())) {
        m_dirPath += QDir::separator();
    }

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
}

void FolderModel::clear()
{
    BaseModel::clear();
}

void FolderModel::deleteItem(const KFileItem & fileItem)
{
    for (int i = 0; i < size(); i++) {
        Item item = itemAt(i);

        if (fileItem.url().url() == item.data.toString()) {
            removeAt(i);
        }
    }
}

void FolderModel::newItems(const KFileItemList &items)
{
    foreach (const KFileItem &item, items) {
        if (item.isDesktopFile()) {
            addUrl(item.url());
        } else {
            add(
                item.name(),
                item.mimeComment(),
                KIcon(item.iconName()),
                item.url().url()
            );
        }
    }
}

} // namespace Models

#include "FolderModel.moc"
