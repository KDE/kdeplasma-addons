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
    emit itemActivated(index);
}

// ActionListViewMergedModel

ActionListViewMergedModel::ActionListViewMergedModel()
{
}

ActionListViewMergedModel::~ActionListViewMergedModel()
{
}

void ActionListViewMergedModel::modelUpdated()
{
    emit updated();
}

void ActionListViewMergedModel::modelItemInserted(int modelIndex)
{
    if (!sender()) return;
    
    int model = m_models.indexOf((ActionListViewModel *)sender());
    if (model == -1) return;
    
    int index;
    fromChildCoordinates(index, model, modelIndex);
    emit itemInserted(index);
}

void ActionListViewMergedModel::modelItemDeleted(int modelIndex)
{
    if (!sender()) return;
    
    int model = m_models.indexOf((ActionListViewModel *)sender());
    if (model == -1) return;
    
    int index;
    fromChildCoordinates(index, model, modelIndex);
    emit itemDeleted(index);
}

void ActionListViewMergedModel::modelItemAltered(int modelIndex)
{
    if (!sender()) return;
    
    int model = m_models.indexOf((ActionListViewModel *)sender());
    if (model == -1) return;
    
    int index;
    fromChildCoordinates(index, model, modelIndex);
    emit itemAltered(index);
}

QString ActionListViewMergedModel::description(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);
    
    if (model == -1 || modelIndex == -1) return "";
    return m_models.at(model)->description(modelIndex);

}

KIcon * ActionListViewMergedModel::icon(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);
    
    if (model == -1) return NULL;
    if (modelIndex == -1) return m_modelsMetadata.at(model).second;
    return m_models.at(model)->icon(modelIndex);
}

bool ActionListViewMergedModel::isCategory(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);
    
    if (model == -1) return false;
    if (model != -1 && modelIndex == -1) return true;
    return m_models.at(model)->isCategory(modelIndex);
}

void ActionListViewMergedModel::activated(int index)
{
    emit itemActivated(index);
    
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);
    
    if (model == -1 || modelIndex == -1) return;
    
    m_models.at(model)->activated(modelIndex);
}

QString ActionListViewMergedModel::title(int index) const
{
    int model, modelIndex;
    toChildCoordinates(index, model, modelIndex);
    
    if (model == -1) return "Error";
    if (modelIndex == -1) return m_modelsMetadata.at(model).first;
    return m_models.at(model)->title(modelIndex);
}

void ActionListViewMergedModel::toChildCoordinates(int index, int & model, int & modelIndex) const
{
    model = 0; modelIndex = 0;
    foreach (ActionListViewModel * m, m_models) {
        if (index <= m->size()) {
            modelIndex = index - 1;
            return;
        } else {
            index -= m->size() + 1;
            ++model;
        }
    }
    model = -1;
    modelIndex = -1;
}

void ActionListViewMergedModel::fromChildCoordinates(int & index, int model, int modelIndex) const
{
    index = -1;
    if (model >= m_models.size()) return;
    if (modelIndex >= m_models.at(model)->size()) return;
    
    index = 0;
    foreach (ActionListViewModel * m, m_models) {
        if (model > 0) {
            index += m->size() + 1;
        } else {
            index += modelIndex + 1;
            return;
        }
        --model;
    }
    index = -1;
}

void ActionListViewMergedModel::addModel(KIcon * icon, QString title, ActionListViewModel * model)
{
    if (!model) return;
    m_models.append(model);
    m_modelsMetadata.append(QPair< QString, KIcon * >(title, icon));
    
    connect(model, SIGNAL(updated()),         this, SLOT(modelUpdated()));
    connect(model, SIGNAL(itemInserted(int)), this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),  this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),  this, SLOT(modelItemAltered(int)));

    emit updated();
}

int ActionListViewMergedModel::modelCount() const
{
    return m_models.size();
}


int ActionListViewMergedModel::size() const
{
    int size = m_models.size();
    kDebug() << size << "\n";
    foreach (ActionListViewModel * model, m_models) {
        size += model->size();
    }
    return size;
}


}

#include "ActionListViewModels.moc"
