/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOT_PASSAGEWAY_VIEW_MODELS_H_
#define LANCELOT_PASSAGEWAY_VIEW_MODELS_H_

#include <lancelot/lancelot_export.h>

#include "ActionListViewModels.h"

namespace Lancelot
{

class LANCELOT_EXPORT PassagewayViewModel: public ActionListViewModel {
public:
    PassagewayViewModel();
    virtual ~PassagewayViewModel();
    virtual PassagewayViewModel * child(int index) = 0;
    virtual QString modelTitle() const = 0;
    virtual QIcon modelIcon() const = 0;

    virtual QMimeData * modelMimeData();
};

class LANCELOT_EXPORT PassagewayViewModelProxy: public PassagewayViewModel {
    Q_OBJECT
public:
    PassagewayViewModelProxy(ActionListViewModel * model,
            QString title = QString(), QIcon icon = QIcon());

    // PassagewayViewModel
    L_Override virtual PassagewayViewModel * child(int index);
    L_Override virtual QString modelTitle() const;
    L_Override virtual QIcon modelIcon()  const;

    // ActionListViewModel
    L_Override virtual QString title(int index) const;
    L_Override virtual QString description(int index) const;
    L_Override virtual QIcon icon(int index) const;
    L_Override virtual bool isCategory(int index) const;
    L_Override virtual bool hasContextActions(int index) const;
    L_Override virtual void setContextActions(int index, QMenu * menu);
    L_Override virtual void contextActivate(int index, QAction * context);
    L_Override virtual QMimeData * mimeData(int index) const;
    L_Override virtual void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);

    L_Override virtual int size() const;

Q_SIGNALS:
    void itemActivated(int index);

    void updated();
    void itemInserted(int index);
    void itemDeleted(int index);
    void itemAltered(int index);

protected:
    /** Models should reimplement this function. It is invoked when
     *  an item is activated, before the itemActivated signal is emitted */
    void activate(int index);

private:
    ActionListViewModel * m_model;
    QString m_modelTitle;
    QIcon m_modelIcon;
};

} // namespace Lancelot

#endif /* LANCELOT_PASSAGEWAY_VIEW_MODELS_H_ */

