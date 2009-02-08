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

#include "MergedActionListModel.h"
#include <KDebug>

namespace Lancelot {

MergedActionListModel::MergedActionListModel()
  : m_hideEmptyModels(true)
{
}

MergedActionListModel::~MergedActionListModel()
{
}

void MergedActionListModel::modelUpdated()
{
    emit updated();
}

void MergedActionListModel::modelItemInserted(int modelIndex)
{
    if (!sender()) return;

    ActionListModel * m = (ActionListModel *)sender();
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

void MergedActionListModel::modelItemDeleted(int modelIndex)
{
    if (!sender()) return;

    ActionListModel * m = (ActionListModel *)sender();
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

void MergedActionListModel::modelItemAltered(int modelIndex)
{
    if (!sender()) return;

    int model = m_models.indexOf((ActionListModel *)sender());
    if (model == -1) return;

    int index;
    fromChildCoordinates(index, model, modelIndex);
    emit itemAltered(index);
}

QString MergedActionListModel::description(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return "";
    return m_models.at(model)->description(modelIndex);

}

QIcon MergedActionListModel::icon(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return QIcon();
    if (modelIndex == -1) return m_modelsMetadata.at(model).second;
    return m_models.at(model)->icon(modelIndex);
}

QMimeData * MergedActionListModel::mimeData(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return NULL;
    if (modelIndex == -1) return modelMimeData(model);
    return m_models.at(model)->mimeData(modelIndex);
}

void MergedActionListModel::dataDropped(int index, Qt::DropAction action)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelDataDropped(model, action);
    return m_models.at(model)->dataDropped(modelIndex, action);
}

void MergedActionListModel::modelDataDropped(int index, Qt::DropAction action)
{
    Q_UNUSED(index);
    Q_UNUSED(action);
}

void MergedActionListModel::setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelDropActions(model, actions, defaultAction);
    return m_models.at(model)->setDropActions(modelIndex, actions, defaultAction);
}

void MergedActionListModel::setModelDropActions(int index,
        Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::IgnoreAction;
    defaultAction = Qt::IgnoreAction;
}

QMimeData * MergedActionListModel::modelMimeData(int index) const
{
    return NULL;
}

bool MergedActionListModel::hasModelContextActions(int index) const
{
    return false;
}

void MergedActionListModel::setModelContextActions(int index, Lancelot::PopupMenu * menu)
{
    // do nothing
}

void MergedActionListModel::modelContextActivate(int index, QAction * context)
{
    // do nothing
}

bool MergedActionListModel::isCategory(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (model != -1 && modelIndex == -1) return true;
    return m_models.at(model)->isCategory(modelIndex);
}

void MergedActionListModel::activate(int index)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return;

    m_models.at(model)->activated(modelIndex);
}

QString MergedActionListModel::title(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return "Error";
    if (modelIndex == -1) return m_modelsMetadata.at(model).first;
    return m_models.at(model)->title(modelIndex);
}

bool MergedActionListModel::hasContextActions(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (modelIndex == -1) return hasModelContextActions(model);
    return m_models.at(model)->hasContextActions(modelIndex);
}

void MergedActionListModel::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelContextActions(model, menu);
    m_models.at(model)->setContextActions(modelIndex, menu);
}

void MergedActionListModel::contextActivate(int index, QAction * context)
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelContextActivate(model, context);
    m_models.at(model)->contextActivate(modelIndex, context);
}

void MergedActionListModel::toChildCoordinates(int index, int & model, int & modelIndex) const
{
    model = 0; modelIndex = 0;
    foreach (ActionListModel * m, m_models) {
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

void MergedActionListModel::fromChildCoordinates(int & index, int model, int modelIndex) const
{
    index = -1;
    if (model >= m_models.size()) return;
    if (modelIndex >= m_models.at(model)->size()) return;

    index = 0;
    foreach (ActionListModel * m, m_models) {
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

void MergedActionListModel::addModel(QIcon icon, const QString & title, ActionListModel * model)
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

ActionListModel * MergedActionListModel::modelAt(int index)
{
    return m_models.at(index);
}

void MergedActionListModel::removeModel(int index)
{
    if (index < 0 || index >= m_models.size()) {
        return;
    }

    ActionListModel * model = m_models.takeAt(index);
    m_modelsMetadata.removeAt(index);
    model->disconnect(this);
    emit updated();
}

int MergedActionListModel::modelCount() const
{
    return m_models.size();
}


int MergedActionListModel::size() const
{
    int result = 0;
    foreach (ActionListModel * model, m_models) {
        if (m_hideEmptyModels && model->size() == 0) continue; // We will not show empty models
        result += model->size() + 1;
    }
    return result;
}

bool MergedActionListModel::hideEmptyModels() const
{
    return m_hideEmptyModels;
}

void MergedActionListModel::setHideEmptyModels(bool hide)
{
    if (m_hideEmptyModels == hide) return;
    m_hideEmptyModels = hide;
    emit updated();
}

} // namespace Lancelot

#include "MergedActionListModel.moc"

