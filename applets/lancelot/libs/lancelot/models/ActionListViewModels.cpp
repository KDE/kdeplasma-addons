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

KIcon * ActionListViewModel::icon(int index) const
{
    Q_UNUSED(index);
    return NULL;
}

bool ActionListViewModel::isCategory(int index) const {
    Q_UNUSED(index);
    return false;
}

void ActionListViewModel::activated(int index) {
    activate(index);
    emit itemActivated(index);
}

// StandardActionListViewModel

StandardActionListViewModel::StandardActionListViewModel()
{
}

StandardActionListViewModel::~StandardActionListViewModel()
{
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

KIcon * StandardActionListViewModel::icon(int index) const
{
    if (index >= m_items.size()) return NULL;
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
    emit itemInserted(m_items.size() - 1);
}

void StandardActionListViewModel::add(const QString & title, const QString & description, KIcon * icon, const QVariant & data)
{
    m_items.append(Item(title, description, icon, data));
    emit itemInserted(m_items.size() - 1);
}

void StandardActionListViewModel::set(int index, const Item & item)
{
    if (index >= m_items.size()) return;
    m_items[index] = item;
    emit itemAltered(index);
}

void StandardActionListViewModel::set(int index, const QString & title, const QString & description, KIcon * icon, const QVariant & data)
{
    if (index >= m_items.size()) return;
    m_items[index] = Item(title, description, icon, data);
    emit itemAltered(index);
}

void StandardActionListViewModel::removeAt(int index)
{
    m_items.removeAt(index);
    emit itemDeleted(index);
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
        emit itemInserted(index - 1); // We insert the model name too // TODO: Check if this really works
    }
    emit itemInserted(index);
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
        emit itemDeleted(index + 1); // We delete the model name too // TODO: Check if this really works
    }
    emit itemDeleted(index);
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

KIcon * MergedActionListViewModel::icon(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);

    if (model == -1) return NULL;
    if (modelIndex == -1) return m_modelsMetadata.at(model).second;
    return m_models.at(model)->icon(modelIndex);
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

void MergedActionListViewModel::addModel(KIcon * icon, QString title, ActionListViewModel * model)
{
    if (!model) return;
    m_models.append(model);
    m_modelsMetadata.append(QPair< QString, KIcon * >(title, icon));

    connect(model, SIGNAL(updated()),         this, SLOT(modelUpdated()));
    connect(model, SIGNAL(itemInserted(int)), this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),  this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),  this, SLOT(modelItemAltered(int)));

    if (m_hideEmptyModels && model->size() == 0) return; // We will not show empty models

    emit updated();
}

int MergedActionListViewModel::modelCount() const
{
    return m_models.size();
}


int MergedActionListViewModel::size() const
{
    int size;
    foreach (ActionListViewModel * model, m_models) {
        if (m_hideEmptyModels && model->size() == 0) continue; // We will not show empty models
        size += model->size() + 1;
    }
    return size;
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

}

#include "ActionListViewModels.moc"
