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

#ifndef LANCELOT_ACTION_TREE_MODEL_PROXY_H
#define LANCELOT_ACTION_TREE_MODEL_PROXY_H

#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionTreeModel.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot
{

class LANCELOT_EXPORT ActionTreeModelProxy: public ActionTreeModel {
    Q_OBJECT
public:
    explicit ActionTreeModelProxy(ActionListModel * model,
            QString title = QString(), QIcon icon = QIcon());
    ~ActionTreeModelProxy();

    ActionListModel * model() const;

    // ActionTreeModel
    L_Override ActionTreeModel * child(int index);
    L_Override QString selfTitle() const;
    L_Override QIcon selfIcon()  const;

    // ActionListModel
    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override bool isCategory(int index) const;
    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);
    L_Override QMimeData * mimeData(int index) const;
    L_Override void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override bool dataDropAvailable(int where, const QMimeData * mimeData);
    L_Override void dataDropped(int where, const QMimeData * mimeData);
    L_Override void dataDragFinished(int index, Qt::DropAction action);

    L_Override int size() const;

Q_SIGNALS:
    void itemActivated(int index);

    void updated();
    void itemInserted(int index);
    void itemDeleted(int index);
    void itemAltered(int index);

protected:
    L_Override void activate(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_TREE_MODEL_PROXY_H */

