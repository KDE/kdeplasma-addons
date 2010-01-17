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

#ifndef LANCELOTAPP_MODELS_APPLICATIONS_H
#define LANCELOTAPP_MODELS_APPLICATIONS_H

#include <lancelot/lancelot_export.h>

#include "BaseModel.h"
#include <lancelot/models/ActionTreeModel.h>

namespace Lancelot {
namespace Models {

/**
 * Applications data model.
 */
class LANCELOT_EXPORT Applications : public Lancelot::ActionTreeModel {
    Q_OBJECT
public:
    /**
     * Creates a new instance of Applications
     * @param root category to show applications for
     * @param title the title of the data model
     * @param icon the icon of the data model
     * @param flat if false, subcategories will behave like normal items
     */
    explicit Applications(QString root = "", QString title = "", QIcon icon = QIcon(), bool flat = false);

    /**
     * Destroys this Applications instance
     */
    virtual ~Applications();

    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override bool isCategory(int index) const;

    L_Override int size() const;

    L_Override ActionTreeModel * child(int index);
    L_Override QString selfTitle() const;
    L_Override QIcon selfIcon()  const;

    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);

    L_Override QMimeData * mimeData(int index) const;
    L_Override void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override QMimeData * selfMimeData() const;

public Q_SLOTS:
    virtual void activate(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_APPLICATIONS_H */
