/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef REMEMBERTHEMILK_PLASMOID_H
#define REMEMBERTHEMILK_PLASMOID_H
 
#include <Plasma/PopupApplet>

#include <QGraphicsLinearLayout>
#include <QList>

#include "taskitem.h"
#include "taskeditor.h"
#include "taskitemdelegate.h"
#include "tasksortfilter.h"

namespace Plasma {
  class TabBar;
  class Label;
  class LineEdit;
  class TreeView;
  class ServiceJob;
}

namespace Ui
{
    class AuthWidget;
    class GeneralWidget;
}

class QSizeF;
class QSortFilterProxyModel;
class TaskModel;

class RememberTheMilkPlasmoid : public Plasma::PopupApplet
{
  Q_OBJECT
  public:
    RememberTheMilkPlasmoid(QObject *parent, const QVariantList &args);
    ~RememberTheMilkPlasmoid();

    virtual void resizeEvent(QGraphicsSceneResizeEvent* event);
    QGraphicsWidget* graphicsWidget();
    void init();

  public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
    
    void showTaskEditor(QModelIndex index);
    void onTaskEditorHide();
    
    void busyUntil(Plasma::ServiceJob *job);
    void jobFinished(Plasma::ServiceJob* job);
    
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();
    void configFinished();
    void startAuth();
    
    void setSortBy(SortBy sortBy);
    
    void listChanged(int pos);
    void createTask(const QString &task);

  private:
    QString                      m_token;
    QList<qulonglong>            m_lists;
    bool                         m_authenticated;
    QList<Plasma::ServiceJob*>   m_busyUntil;
    SortBy                       m_sortBy;
    TaskEditor                  *m_taskEditor;
    Plasma::DataEngine          *m_engine;
    Plasma::Service             *m_taskService;
    Plasma::Label               *m_priorityLabel;
    Plasma::TabBar              *m_categoriesBar;
    Plasma::TreeView            *m_tasksView;
    Plasma::LineEdit            *m_addTaskLine;
    QGraphicsLinearLayout       *m_mainLayout;
    TaskModel                   *m_model;
    TaskItemDelegate            *m_delegate;
    QGraphicsWidget             *m_graphicsWidget;
    Plasma::LineEdit            *m_searchLine;
    TaskSortFilter              *m_filterModel;
    
    Plasma::Service             *m_authService;
    Ui::AuthWidget              *m_authWidgetUi;
    QWidget                     *m_authWidget;

    Ui::GeneralWidget           *m_generalOptionsUi;
    QWidget                     *m_generalOptions;
    
};
 
// This links the .desktop file to the applet
K_EXPORT_PLASMA_APPLET(rtm, RememberTheMilkPlasmoid)

#endif // REMEMBERTHEMILK-PLASMOID_H
