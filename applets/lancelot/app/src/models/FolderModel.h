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

#ifndef LANCELOTAPP_MODELS_FOLDERMODEL_H
#define LANCELOTAPP_MODELS_FOLDERMODEL_H

#include "BaseModel.h"
#include <QDir>
#include <KDirLister>
#include <KFileItem>

namespace Models {

class FolderModel : public BaseModel {
    Q_OBJECT
public:
    explicit FolderModel(QString dirPath, QDir::SortFlags sort = QDir::NoSort);
    virtual ~FolderModel();

protected:
    void load();

protected:
    KDirLister * m_dirLister;
    QString m_dirPath;
    QDir::SortFlags m_sort;

protected Q_SLOTS:
    void clear();
    void deleteItem(const KFileItem & fileItem);
    void newItems(const KFileItemList & items);
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_FOLDERMODEL_H */
