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

#include "rememberthemilk-plasmoid.h"

// Qt Includes
#include <QPushButton>
#include <QCheckBox>
#include <QTreeView>

// KDE Includes
#include <KConfigDialog>
#include <KDebug> 
#include <KToolInvocation>

// Plasma Includes
#include <Plasma/Theme>
#include <Plasma/Service>
#include <Plasma/Animator>
#include <Plasma/TreeView>
#include <Plasma/TabBar>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/ServiceJob>

// Local
#include "ui_authenticate.h"
#include "ui_general.h"
#include "taskmodel.h"
#include "tasksortfilter.h"
#include <KLineEdit>

RememberTheMilkPlasmoid::RememberTheMilkPlasmoid(QObject* parent, const QVariantList& args)
    : Plasma::PopupApplet(parent, args),
    m_authenticated(false),
    m_sortBy(SortDue),
    m_taskEditor(0),
    m_engine(0),
    m_taskService(0),
    m_graphicsWidget(0),
    m_authService(0),
    m_authWidgetUi(new Ui::AuthWidget),
    m_generalOptionsUi(new Ui::GeneralWidget)
{
    setBackgroundHints(DefaultBackground);
    setPopupIcon("view-pim-journal");  // TODO: Create/Find better item
    m_authWidget = new QWidget();
    m_authWidgetUi->setupUi(m_authWidget);
    m_generalOptions = new QWidget();
    connect(m_authWidgetUi->signupLabel, SIGNAL(leftClickedUrl(const QString&)), KToolInvocation::self(), SLOT(invokeBrowser(const QString&)));
    m_generalOptionsUi->setupUi(m_generalOptions);
    m_generalOptionsUi->sortType->addItem(i18n("Due Date"));
    m_generalOptionsUi->sortType->addItem(i18n("Priority"));
}
 
RememberTheMilkPlasmoid::~RememberTheMilkPlasmoid() {
    if (hasFailedToLaunch()) {
      kDebug() << "failed to launch";
    }
  delete m_authWidget;
  delete m_authWidgetUi;
  
  delete m_generalOptions;
  delete m_generalOptionsUi;
}

void RememberTheMilkPlasmoid::init() {
  setAspectRatioMode(Plasma::IgnoreAspectRatio);
  
  graphicsWidget();
  
  if (!m_engine || !m_engine->isValid()) {
    return Plasma::Applet::init();
  }
  
  m_engine->connectSource("Auth", this);
  
  m_authService = m_engine->serviceForSource("Auth");
  connect(m_authService, SIGNAL(finished(Plasma::ServiceJob*)), SLOT(jobFinished(Plasma::ServiceJob*)));
  
  if (m_token.isNull())
    setConfigurationRequired(true, i18n("Authentication to Remember The Milk needed"));
  else {
    KConfigGroup cg = m_authService->operationDescription("AuthWithToken");
    cg.writeEntry("token", m_token);
    busyUntil(m_authService->startOperationCall(cg)); 
    busyUntil(0); // Sets busy until we manually call jobFinished(0). Busy until first tasks refresh
  }
  
  QString sortBy = config().readEntry("SortBy").toLower();  
  kDebug() << "Config says sort by " << sortBy;
  if (sortBy == "date" || sortBy == "due")
    setSortBy(SortDue);
  else if (sortBy == "priority")
    setSortBy(SortPriority);
  else
    setSortBy(SortDue); // Default
  
  Plasma::Applet::init();
}


void RememberTheMilkPlasmoid::startAuth()
{
  KConfigGroup cg = m_authService->operationDescription("Login");
  busyUntil(m_authService->startOperationCall(cg));
  busyUntil(0); // Sets busy until we manually call jobFinished(0). Busy until first tasks refresh
  m_authenticated = false;  
}

void RememberTheMilkPlasmoid::configAccepted() {
  switch(m_generalOptionsUi->sortType->currentIndex()) {
    case 0:
      setSortBy(SortDue);
      break;
    case 1:
      setSortBy(SortPriority);
      break;
  }
}

void RememberTheMilkPlasmoid::configFinished()
{
  m_authWidget->setParent(0); // We don't want these widgets destroyed along with the configuration window
  m_generalOptions->setParent(0); // They are reused later
}

void RememberTheMilkPlasmoid::createConfigurationInterface(KConfigDialog* parent) {
  connect(parent, SIGNAL(finished()), this, SLOT(configFinished()));
  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
  connect(m_authWidgetUi->authenticate, SIGNAL(clicked(bool)), this, SLOT(startAuth()));
  
  m_generalOptionsUi->sortType->setCurrentIndex(m_sortBy);
  
  KPageWidgetItem* general = parent->addPage(m_generalOptions, i18n("General"), "configure", i18n("General Configuration Options"));
  KPageWidgetItem* auth = parent->addPage(m_authWidget, i18n("Authentication"), icon(), i18n("Remember The Milk Authentication"));
  
  if (!m_authenticated)
    parent->setCurrentPage(auth);
  else
    parent->setCurrentPage(general);
}

void RememberTheMilkPlasmoid::setSortBy(SortBy sortBy)
{
  m_filterModel->setSortBy(sortBy);
  m_model->setDropType(sortBy);
  switch (sortBy) {
    case SortPriority:
      config().writeEntry("SortBy", "Priority");
      m_delegate->setSmallTasks(false);
      break;
    case SortDue:
      config().writeEntry("SortBy", "Date");
      m_delegate->setSmallTasks(true);  
      break;
  }
  
  m_sortBy = sortBy;
}

void RememberTheMilkPlasmoid::dataUpdated(const QString& name, const Plasma::DataEngine::Data& data) {
  if (name == "Auth") {
    m_authenticated = data.value("ValidToken").toBool();
    kDebug() << "Auth: " << m_authenticated;
    
    if (m_authenticated) {
      m_authWidgetUi->authStatus->setText(i18n("Authenticated"));
      m_authWidgetUi->kled->setState(KLed::On);
      m_authWidgetUi->kled->setColor(Qt::green);
    } else {
      m_authWidgetUi->authStatus->setText(i18n("Not Authenticated"));
      m_authWidgetUi->kled->setState(KLed::Off); 
      m_authWidgetUi->kled->setColor(Qt::red);
    }
    
    if (m_authenticated) {
      setConfigurationRequired(false);
      m_token = data.value("Token").toString();
      config().writeEntry("token", m_token);
      m_engine->connectSource("Lists", this);
      m_engine->connectSource("Tasks", this);
    }
    else if (m_categoriesBar->count() == 1 && m_lists.isEmpty())
      m_categoriesBar->nativeWidget()->setTabText(0, i18n("Login Failed. Please try again."));
  }
  else if (name == "Lists") {
    if (m_categoriesBar->count() == 1 && m_lists.isEmpty())
      m_categoriesBar->removeTab(0); // remove "refreshing" tab;
    //while (m_categoriesBar->count())
    //  m_categoriesBar->removeTab(0);
    //m_lists.clear(); //FIXME: What happens when a list is deleted?
    foreach(const QString &key, data.keys()) {
      if (!m_lists.contains(key.toULongLong())) {
        m_categoriesBar->addTab(data.value(key).toString());
        m_lists.append(key.toULongLong());
      }
      m_model->listUpdate(key.toULongLong());
    }
    int current = config().readEntry("CurrentList").toInt();
    if (m_categoriesBar->count() >= current) {
      m_categoriesBar->setCurrentIndex(current);
      listChanged(current); // doesn't always update.. i.e if current is actually the current
    }

   }
  else if (name == "Tasks") {
    if (data.keys().count() > 0)
      jobFinished(0); // No longer busy.
  }
}

void RememberTheMilkPlasmoid::listChanged(int pos)
{
  config().writeEntry("CurrentList", pos);
  m_model->switchToList(m_lists.at(pos));
}

void RememberTheMilkPlasmoid::createTask(const QString& task)
{
    if (!m_taskService) {
      m_taskService = m_engine->serviceForSource("Tasks");
      connect(m_taskService, SIGNAL(finished(Plasma::ServiceJob*)), SLOT(jobFinished(Plasma::ServiceJob*)));
    }
    KConfigGroup cg = m_taskService->operationDescription("create");
    cg.writeEntry("task", task);
    kDebug() << "QString::number(m_model->currentList()->id)";
    cg.writeEntry("listid", m_model->currentList()->id);
    busyUntil(m_taskService->startOperationCall(cg));
}

void RememberTheMilkPlasmoid::onTaskEditorHide() {
  m_tasksView->nativeWidget()->setEnabled(true);
  m_priorityLabel->setText(i18n("Remember The Milk Tasks"));
}

void RememberTheMilkPlasmoid::showTaskEditor(QModelIndex index) {
  if (index.data(Qt::RTMItemType).toInt() != RTMTaskItem) // We have a header rather than a task.
    return;
  m_taskEditor->setModelIndex(index);
  m_taskEditor->hide();
  m_taskEditor->show();
  m_tasksView->nativeWidget()->setEnabled(false);
  m_taskEditor->setEnabled(true);
  m_taskEditor->startAnimation(m_tasksView->size());
  m_priorityLabel->setText(i18n("Editing Task: ") + index.data(Qt::RTMNameRole).toString());
  
  geometryChanged();
}

void RememberTheMilkPlasmoid::busyUntil(Plasma::ServiceJob* job)
{
  setBusy(true);
  m_busyUntil.append(job);
}

void RememberTheMilkPlasmoid::jobFinished(Plasma::ServiceJob* job)
{
  if (m_busyUntil.contains(job)) {
    m_busyUntil.removeAll(job);
  }
  if (m_busyUntil.isEmpty())
    setBusy(false);
}

void RememberTheMilkPlasmoid::resizeEvent(QGraphicsSceneResizeEvent* event) {
  if (m_taskEditor)
    m_taskEditor->resize(m_tasksView->size());
  Plasma::Applet::resizeEvent(event);
}

QGraphicsWidget* RememberTheMilkPlasmoid::graphicsWidget() {
  if (m_graphicsWidget)
    return m_graphicsWidget;
  
  m_engine = dataEngine("rtm");
  
  if (!m_engine || !m_engine->isValid()) {
      setFailedToLaunch(true, i18n("Failed to load the Remember The Milk DataEngine"));
      return m_graphicsWidget;
  }
  
  m_graphicsWidget = new QGraphicsWidget(this);
  
  m_model = new TaskModel(m_engine, this);
  connect(m_model, SIGNAL(jobStarted(Plasma::ServiceJob*)), SLOT(busyUntil(Plasma::ServiceJob*)));
  connect(m_model, SIGNAL(jobFinished(Plasma::ServiceJob*)), SLOT(jobFinished(Plasma::ServiceJob*)));

  m_priorityLabel = new Plasma::Label(this);
  m_priorityLabel->setAlignment(Qt::AlignHCenter);
  m_priorityLabel->setText(i18n("Remember The Milk Tasks"));

  m_categoriesBar = new Plasma::TabBar(this);
  m_categoriesBar->addTab(i18n("No Data Yet. Refreshing..."));
  
  connect(m_categoriesBar, SIGNAL(currentChanged(int)), SLOT(listChanged(int)));

  QPalette palette;
  palette.setBrush(QPalette::Background, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
  palette.setBrush(QPalette::Text, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));


  m_delegate = new TaskItemDelegate(this);

  m_filterModel = new TaskSortFilter(m_model, this);
  connect(m_model, SIGNAL(modelUpdated()), m_filterModel, SLOT(listChanged()));

  m_tasksView = new Plasma::TreeView(this);
  m_tasksView->installEventFilter(m_delegate);
  m_tasksView->setModel(m_filterModel);
  m_tasksView->nativeWidget()->setItemDelegate(m_delegate);
  m_tasksView->nativeWidget()->header()->setHidden(true);
  m_tasksView->nativeWidget()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_tasksView->nativeWidget()->setRootIsDecorated(false);
  m_tasksView->nativeWidget()->setIndentation(0);
  m_tasksView->nativeWidget()->viewport()->setAutoFillBackground(false);
  m_tasksView->nativeWidget()->viewport()->setPalette( palette );
  m_tasksView->nativeWidget()->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tasksView->nativeWidget()->setDragEnabled(true);
  m_tasksView->nativeWidget()->setAcceptDrops(true);
  m_tasksView->nativeWidget()->viewport()->setAcceptDrops(true);
  m_tasksView->nativeWidget()->setDropIndicatorShown(false);
  m_tasksView->nativeWidget()->setDragDropMode(QAbstractItemView::DragDrop);
  m_tasksView->nativeWidget()->setSortingEnabled(true);
  m_tasksView->nativeWidget()->sortByColumn(0, Qt::AscendingOrder);
  m_tasksView->nativeWidget()->expandAll();

  connect(m_tasksView->nativeWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(showTaskEditor(QModelIndex)));

  m_searchLine = new Plasma::LineEdit(this);
  m_searchLine->nativeWidget()->setClearButtonShown(true);
  m_searchLine->nativeWidget()->setClickMessage(i18n("Filter Tasks..."));
  connect(m_searchLine->nativeWidget(), SIGNAL(textChanged(QString)), m_filterModel, SLOT(setFilterWildcard(QString)));

  m_addTaskLine = new Plasma::LineEdit(this); // must wait for plasma theming (4.3?) to get a themed lineedit

  m_addTaskLine->nativeWidget()->setClearButtonShown(true);
  m_addTaskLine->nativeWidget()->setClickMessage("Create New Task...");

  connect(m_addTaskLine->nativeWidget(), SIGNAL(returnPressed(QString)), m_addTaskLine->nativeWidget(), SLOT(clear()));
  connect(m_addTaskLine->nativeWidget(), SIGNAL(returnPressed(QString)), this, SLOT(createTask(QString)));

  kDebug() << "Creating layout";

  m_mainLayout = new QGraphicsLinearLayout(Qt::Vertical, m_graphicsWidget);

  m_mainLayout->addItem(m_priorityLabel);
  m_mainLayout->addItem(m_categoriesBar);
  m_mainLayout->addItem(m_searchLine);
  m_mainLayout->addItem(m_tasksView);
  m_mainLayout->addItem(m_addTaskLine);
  
  m_graphicsWidget->setLayout(m_mainLayout);
  m_graphicsWidget->setMinimumSize(250, 300);
  m_graphicsWidget->setPreferredSize(300, 500);

  KConfigGroup cg = config();
  m_token = cg.readEntry("token");
  kDebug() << "Token from config: " << m_token;
 
  m_taskEditor = new TaskEditor(m_engine, m_tasksView);
  m_taskEditor->hide();
  connect(m_taskEditor, SIGNAL(requestDiscardChanges()), this, SLOT(onTaskEditorHide()));
  connect(m_taskEditor, SIGNAL(requestSaveChanges()), this, SLOT(onTaskEditorHide()));
  connect(m_taskEditor, SIGNAL(jobStarted(Plasma::ServiceJob*)), SLOT(busyUntil(Plasma::ServiceJob*)));
  connect(m_taskEditor, SIGNAL(jobFinished(Plasma::ServiceJob*)), SLOT(jobFinished(Plasma::ServiceJob*)));
  
  return m_graphicsWidget;
}

#include "rememberthemilk-plasmoid.moc"
