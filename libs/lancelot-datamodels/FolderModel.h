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

#ifndef LANCELOTAPP_MODELS_FOLDERMODEL_H
#define LANCELOTAPP_MODELS_FOLDERMODEL_H

#include <lancelot/lancelot_export.h>

#include "BaseModel.h"
#include <QDir>
#include <KDirLister>
#include <KFileItem>

namespace Lancelot {
namespace Models {

class FolderModelPrivate;

class LANCELOT_EXPORT FolderModel : public BaseModel {
    Q_OBJECT
public:
    explicit FolderModel(QString dirPath, QDir::SortFlags sort = QDir::NoSort);
    virtual ~FolderModel();

    L_Override bool dataDropAvailable(int where, const QMimeData * mimeData);
    L_Override void dataDropped(int where, const QMimeData * mimeData);

    QString dirPath() const;

protected:
    void load();

protected Q_SLOTS:
    void clear();
    void save();
    void update();

private:
    friend class FolderModelPrivate;
    class FolderModelPrivate * const d;

    Q_PRIVATE_SLOT(d, void deleteItem(const KFileItem & fileItem))
    Q_PRIVATE_SLOT(d, void newItems(const KFileItemList & fileItems))
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_FOLDERMODEL_H */
