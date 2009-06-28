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

namespace Lancelot {

class MergedActionListModel::Private {
public:
    void toChildCoordinates(int index, int & model, int & modelIndex) const;
    void fromChildCoordinates(int & index, int model, int modelIndex) const;

    QList < ActionListModel * > models;
    QList < QPair < QString, QIcon > > modelsMetadata;
    bool hideEmptyModels;
};

MergedActionListModel::MergedActionListModel()
  : d(new Private)
{
    d->hideEmptyModels = true;
}

MergedActionListModel::~MergedActionListModel()
{
    delete d;
}

void MergedActionListModel::modelUpdated()
{
    emit updated();
}

void MergedActionListModel::modelItemInserted(int modelIndex)
{
    if (!sender()) return;

    ActionListModel * m = (ActionListModel *)sender();
    int model = d->models.indexOf(m);
    if (model == -1) return;

    int index;
    d->fromChildCoordinates(index, model, modelIndex);

    if (d->hideEmptyModels && m->size() == 1) {
        modelUpdated();
    } else {
        emit itemInserted(index);
    }
}

void MergedActionListModel::modelItemDeleted(int modelIndex)
{
    if (!sender()) return;

    ActionListModel * m = (ActionListModel *)sender();
    int model = d->models.indexOf(m);
    if (model == -1) return;

    int index;
    d->fromChildCoordinates(index, model, modelIndex);

    if (d->hideEmptyModels && m->size() == 0) {
        modelUpdated();
    } else {
        emit itemDeleted(index);
    }
}

void MergedActionListModel::modelItemAltered(int modelIndex)
{
    if (!sender()) return;

    int model = d->models.indexOf((ActionListModel *)sender());
    if (model == -1) return;

    int index;
    d->fromChildCoordinates(index, model, modelIndex);
    emit itemAltered(index);
}

QString MergedActionListModel::description(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return "";
    return d->models.at(model)->description(modelIndex);

}

QIcon MergedActionListModel::icon(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return QIcon();
    if (modelIndex == -1) return d->modelsMetadata.at(model).second;
    return d->models.at(model)->icon(modelIndex);
}

QMimeData * MergedActionListModel::mimeData(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return NULL;
    if (modelIndex == -1) return modelMimeData(model);
    return d->models.at(model)->mimeData(modelIndex);
}

void MergedActionListModel::dataDropped(int index, Qt::DropAction action)
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelDataDropped(model, action);
    return d->models.at(model)->dataDropped(modelIndex, action);
}

void MergedActionListModel::modelDataDropped(int index, Qt::DropAction action)
{
    Q_UNUSED(index);
    Q_UNUSED(action);
}

void MergedActionListModel::setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelDropActions(model, actions, defaultAction);
    return d->models.at(model)->setDropActions(modelIndex, actions, defaultAction);
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
    Q_UNUSED(index);
    return NULL;
}

bool MergedActionListModel::hasModelContextActions(int index) const
{
    Q_UNUSED(index);
    return false;
}

void MergedActionListModel::setModelContextActions(int index, Lancelot::PopupMenu * menu)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
    // do nothing
}

void MergedActionListModel::modelContextActivate(int index, QAction * context)
{
    Q_UNUSED(index);
    Q_UNUSED(context);
    // do nothing
}

bool MergedActionListModel::isCategory(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (model != -1 && modelIndex == -1) return true;
    return d->models.at(model)->isCategory(modelIndex);
}

void MergedActionListModel::activate(int index)
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1 || modelIndex == -1) return;

    d->models.at(model)->activated(modelIndex);
}

QString MergedActionListModel::title(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return "Error";
    if (modelIndex == -1) return d->modelsMetadata.at(model).first;
    return d->models.at(model)->title(modelIndex);
}

bool MergedActionListModel::hasContextActions(int index) const
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return false;
    if (modelIndex == -1) return hasModelContextActions(model);
    return d->models.at(model)->hasContextActions(modelIndex);
}

void MergedActionListModel::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return setModelContextActions(model, menu);
    d->models.at(model)->setContextActions(modelIndex, menu);
}

void MergedActionListModel::contextActivate(int index, QAction * context)
{
    int model, modelIndex;
    d->toChildCoordinates(index, model, modelIndex);

    if (model == -1) return;
    if (modelIndex == -1) return modelContextActivate(model, context);
    d->models.at(model)->contextActivate(modelIndex, context);
}

void MergedActionListModel::Private::toChildCoordinates(int index, int & model, int & modelIndex) const
{
    model = 0; modelIndex = 0;
    foreach (ActionListModel * m, models) {
        if (!hideEmptyModels || m->size() != 0) {
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

void MergedActionListModel::Private::fromChildCoordinates(int & index, int model, int modelIndex) const
{
    index = -1;
    if (model >= models.size()) return;
    if (modelIndex >= models.at(model)->size()) return;

    index = 0;
    foreach (ActionListModel * m, models) {
        if (!hideEmptyModels || m->size() != 0) {
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

void MergedActionListModel::addModel(ActionListModel * model)
{
    addModel(model->selfIcon(), model->selfTitle(), model);
}

void MergedActionListModel::addModel(QIcon icon, const QString & title, ActionListModel * model)
{
    if (!model) return;
    d->models.append(model);
    d->modelsMetadata.append(QPair< QString, QIcon >(title, icon));

    connect(model, SIGNAL(updated()),         this, SLOT(modelUpdated()));
    connect(model, SIGNAL(itemInserted(int)), this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),  this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),  this, SLOT(modelItemAltered(int)));

    if (d->hideEmptyModels && model->size() == 0) return; // We will not show empty models

    emit updated();
}

ActionListModel * MergedActionListModel::modelAt(int index)
{
    return d->models.at(index);
}

void MergedActionListModel::removeModel(int index)
{
    if (index < 0 || index >= d->models.size()) {
        return;
    }

    ActionListModel * model = d->models.takeAt(index);
    d->modelsMetadata.removeAt(index);
    model->disconnect(this);
    emit updated();
}

int MergedActionListModel::modelCount() const
{
    return d->models.size();
}


int MergedActionListModel::size() const
{
    int result = 0;
    foreach (ActionListModel * model, d->models) {
        if (d->hideEmptyModels && model->size() == 0) continue; // We will not show empty models
        result += model->size() + 1;
    }
    return result;
}

bool MergedActionListModel::hideEmptyModels() const
{
    return d->hideEmptyModels;
}

void MergedActionListModel::setHideEmptyModels(bool hide)
{
    if (d->hideEmptyModels == hide) return;
    d->hideEmptyModels = hide;
    emit updated();
}

} // namespace Lancelot

#include "MergedActionListModel.moc"

