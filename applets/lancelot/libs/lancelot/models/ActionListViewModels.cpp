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

#include "ActionListViewModels.h"
#include <KDebug>

namespace Lancelot {

// ActionListViewModel

ActionListViewModel::ActionListViewModel()
{
}

ActionListViewModel::~ActionListViewModel()
{
}

QString ActionListViewModel::description(int index) const
{
    Q_UNUSED(index);
    return QString();
}

QIcon ActionListViewModel::icon(int index) const
{
    Q_UNUSED(index);
    return QIcon();
}

QMimeData * ActionListViewModel::mimeData(int index) const
{
    return NULL;
}

void ActionListViewModel::dataDropped(int index, Qt::DropAction action)
{
    Q_UNUSED(index);
    Q_UNUSED(action);
}

void ActionListViewModel::setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::IgnoreAction;
    defaultAction = Qt::IgnoreAction;
}

bool ActionListViewModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

void ActionListViewModel::activated(int index)
{
    activate(index);
    emit itemActivated(index);
}

bool ActionListViewModel::hasContextActions(int index) const
{
    Q_UNUSED(index);
    return false;
}

void ActionListViewModel::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
}

void ActionListViewModel::contextActivate(int index, QAction * context)
{
    Q_UNUSED(index);
    Q_UNUSED(context);
}

void ActionListViewModel::activate(int index)
{
    Q_UNUSED(index);
}

// StandardActionListViewModel

StandardActionListViewModel::StandardActionListViewModel()
    : m_sendEmits(true)
{
}

StandardActionListViewModel::~StandardActionListViewModel()
{
}

void StandardActionListViewModel::setEmitInhibited(bool value)
{
    m_sendEmits = !value;
}

bool StandardActionListViewModel::emitInhibited() const
{
    return !m_sendEmits;
}

QString StandardActionListViewModel::title(int index) const
{
    if (index >= m_items.size()) return "";
    return m_items.at(index).title;
}

QString StandardActionListViewModel::description(int index) const
{
    if (index >= m_items.size()) return "";
    return m_items.at(index).description;
}

QIcon StandardActionListViewModel::icon(int index) const
{
    if (index >= m_items.size()) return QIcon();
    return m_items.at(index).icon;
}

bool StandardActionListViewModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

int StandardActionListViewModel::size() const
{
    return m_items.size();
}

void StandardActionListViewModel::add(const Item & item)
{
    m_items.append(item);
    if (m_sendEmits) {
        emit itemInserted(m_items.size() - 1);
    }
}

void StandardActionListViewModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    add(Item(title, description, icon, data));
}

void StandardActionListViewModel::set(int index, const Item & item)
{
    if (index >= m_items.size()) return;
    m_items[index] = item;
    if (m_sendEmits) {
        emit itemAltered(index);
    }
}

void StandardActionListViewModel::set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    set(index, Item(title, description, icon, data));
}

void StandardActionListViewModel::removeAt(int index)
{
    m_items.removeAt(index);
    if (m_sendEmits) {
        emit itemDeleted(index);
    }
}

void StandardActionListViewModel::clear()
{
    m_items.clear();
    if (m_sendEmits) {
        emit updated();
    }
}

StandardActionListViewModel::Item & StandardActionListViewModel::itemAt(int index)
{
    return m_items[index];
}

// MergedActionListViewModel

MergedActionListViewModel::MergedActionListViewModel()
  : m_hideEmptyModels(true)
{
}

MergedActionListViewModel::~MergedActionListViewModel()
{
}

void MergedActionListViewModel::modelUpdated()
{
    emit updated();
}

void MergedActionListViewModel::modelItemInserted(int modelIndex)
{
    if (!sender()) return;

    ActionListViewModel * m = (ActionListViewModel *)sender();
    int model = m_models.indexOf(m);
    if (model == -1) return;

    int index;
    fromChildCoordinates(index, model, modelIndex);

    if (m_hideEmptyModels && m->size() == 1) {
        modelUpdated();
    } else {
        emit itemInserted(index);
    }
}

void MergedActionListViewModel::modelItemDeleted(int modelIndex)
{
    if (!sender()) return;

    ActionListViewModel * m = (ActionListViewModel *)sender();
    int model = m_models.indexOf(m);
    if (model == -1) return;

    int index;
    fromChildCoordinates(index, model, modelIndex);

    if (m_hideEmptyModels && m->size() == 0) {
        modelUpdated();
    } else {
        emit itemDeleted(index);
    }
}

void MergedActionListViewModel::modelItemAltered(int modelIndex)
{
    if (!sender()) return;

    int model = m_models.indexOf((ActionListViewModel *)sender());
    if (model == -1) return;

    int index;
    fromChildCoordinates(index, model, modelIndex);
    emit itemAltered(index);
}

QString MergedActionListViewModel::description(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return "";
    return m_models.at(model)->description(modelIndex);

}

QIcon MergedActionListViewModel::icon(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return QIcon();
    if (modelIndex == -1) return m_modelsMetadata.at(model).second;
    return m_models.at(model)->icon(modelIndex);
}

QMimeData * MergedActionListViewModel::mimeData(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return NULL;
    if (modelIndex == -1) return modelMimeData(model);
    return m_models.at(model)->mimeData(modelIndex);
}

void MergedActionListViewModel::dataDropped(int index, Qt::DropAction action)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelDataDropped(model, action);
    return m_models.at(model)->dataDropped(modelIndex, action);
}

void MergedActionListViewModel::modelDataDropped(int index, Qt::DropAction action)
{
    Q_UNUSED(index);
    Q_UNUSED(action);
}

void MergedActionListViewModel::setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelDropActions(model, actions, defaultAction);
    return m_models.at(model)->setDropActions(modelIndex, actions, defaultAction);
}

void MergedActionListViewModel::setModelDropActions(int index,
        Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::IgnoreAction;
    defaultAction = Qt::IgnoreAction;
}

QMimeData * MergedActionListViewModel::modelMimeData(int index) const
{
    return NULL;
}

bool MergedActionListViewModel::hasModelContextActions(int index) const
{
    return false;
}

void MergedActionListViewModel::setModelContextActions(int index, Lancelot::PopupMenu * menu)
{
    // do nothing
}

void MergedActionListViewModel::modelContextActivate(int index, QAction * context)
{
    // do nothing
}

bool MergedActionListViewModel::isCategory(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (model != -1 && modelIndex == -1) return true;
    return m_models.at(model)->isCategory(modelIndex);
}

void MergedActionListViewModel::activate(int index)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return;

    m_models.at(model)->activated(modelIndex);
}

QString MergedActionListViewModel::title(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return "Error";
    if (modelIndex == -1) return m_modelsMetadata.at(model).first;
    return m_models.at(model)->title(modelIndex);
}

bool MergedActionListViewModel::hasContextActions(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (modelIndex == -1) return hasModelContextActions(model);
    return m_models.at(model)->hasContextActions(modelIndex);
}

void MergedActionListViewModel::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelContextActions(model, menu);
    m_models.at(model)->setContextActions(modelIndex, menu);
}

void MergedActionListViewModel::contextActivate(int index, QAction * context)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelContextActivate(model, context);
    m_models.at(model)->contextActivate(modelIndex, context);
}

void MergedActionListViewModel::toChildCoordinates(int index, int & model, int & modelIndex) const
{
    model = 0; modelIndex = 0;
    foreach (ActionListViewModel * m, m_models) {
        if (!m_hideEmptyModels || m->size() != 0) {
            if (index <= m->size()) {
                modelIndex = index - 1;
                return;
            } else {
                index -= m->size() + 1;
                ++model;
            }
        } else {
            ++model;
        }
    }
    model = -1;
    modelIndex = -1;
}

void MergedActionListViewModel::fromChildCoordinates(int & index, int model, int modelIndex) const
{
    index = -1;
    if (model >= m_models.size()) return;
    if (modelIndex >= m_models.at(model)->size()) return;

    index = 0;
    foreach (ActionListViewModel * m, m_models) {
        if (!m_hideEmptyModels || m->size() != 0) {
            if (model > 0) {
                index += m->size() + 1;
            } else {
                index += modelIndex + 1;
                return;
            }
        }
        --model;
    }
    index = -1;
}

void MergedActionListViewModel::addModel(QIcon icon, const QString & title, ActionListViewModel * model)
{
    if (!model) return;
    m_models.append(model);
    m_modelsMetadata.append(QPair< QString, QIcon >(title, icon));

    connect(model, SIGNAL(updated()),         this, SLOT(modelUpdated()));
    connect(model, SIGNAL(itemInserted(int)), this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),  this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),  this, SLOT(modelItemAltered(int)));

    if (m_hideEmptyModels && model->size() == 0) return; // We will not show empty models

    emit updated();
}

ActionListViewModel * MergedActionListViewModel::modelAt(int index)
{
    return m_models.at(index);
}

void MergedActionListViewModel::removeModel(int index)
{
    if (index < 0 || index >= m_models.size()) {
        return;
    }

    ActionListViewModel * model = m_models.takeAt(index);
    m_modelsMetadata.removeAt(index);
    model->disconnect(this);
    emit updated();
}

int MergedActionListViewModel::modelCount() const
{
    return m_models.size();
}


int MergedActionListViewModel::size() const
{
    int result = 0;
    foreach (ActionListViewModel * model, m_models) {
        if (m_hideEmptyModels && model->size() == 0) continue; // We will not show empty models
        result += model->size() + 1;
    }
    return result;
}

bool MergedActionListViewModel::hideEmptyModels() const
{
    return m_hideEmptyModels;
}

void MergedActionListViewModel::setHideEmptyModels(bool hide)
{
    if (m_hideEmptyModels == hide) return;
    m_hideEmptyModels = hide;
    emit updated();
}

} // namespace Lancelot

#include "ActionListViewModels.moc"

