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

#ifndef LANCELOTAPP_MODELS_SYSTEM_ACTIONS_H
#define LANCELOTAPP_MODELS_SYSTEM_ACTIONS_H

#include <Plasma/RunnerManager>
#include <Plasma/QueryMatch>

#include <lancelot/lancelot_export.h>
#include <lancelot/models/StandardActionTreeModel.h>
#include <lancelot/models/ActionTreeModelProxy.h>

#include "BaseModel.h"
#include "Runner.h"

#include <kworkspace/kdisplaymanager.h>

namespace Lancelot {
namespace Models {

class LANCELOT_EXPORT SystemActions: public Lancelot::StandardActionTreeModel {
    Q_OBJECT
public:
    static SystemActions * self();

    ~SystemActions();

    /**
     * If the specified id represents an action, it
     * will be executed and NULL will be returned.
     * If it is a set of actions, pointer to the model
     * will be returned.
     */
    Lancelot::ActionTreeModel * action(const QString & id, bool execute = true);

    /**
     * @returns a list of action IDs
     */
    QStringList actions() const;

    QString actionTitle(const QString & id) const;
    QIcon actionIcon(const QString & id) const;

    L_Override void load();
    L_Override StandardActionTreeModel * createChild(int index);
    L_Override bool isCategory(int index) const;
    L_Override Lancelot::ActionTreeModel * child(int index);

    void activate(int index);

private:
    explicit SystemActions(Item * root);
    explicit SystemActions();

    class Private;
    Private * const d;

};

class LANCELOT_EXPORT Sessions: public BaseModel {
    Q_OBJECT
public:
    Sessions();
    virtual ~Sessions();

    void load();
    void activate(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_SYSTEM_ACTIONS_H */
