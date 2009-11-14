#include "kdeobservatory.h"

#include <QTimer>
#include <QTimeLine>
#include <QProgressBar>
#include <QListWidgetItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsItemAnimation>

#include <KConfig>
#include <KConfigDialog>
#include <KGlobalSettings>

#include <Plasma/Label>
#include <Plasma/PushButton>

#include "kdeobservatoryconfigviews.h"
#include "kdeobservatoryconfiggeneral.h"
#include "kdeobservatoryconfigprojects.h"

#include "krazycollector.h"
#include "commitcollector.h"

#include "topdevelopersview.h"
#include "commithistoryview.h"
#include "topactiveprojectsview.h"

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args),
  m_transitionTimer(0)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(300, 200);

    // Collector in key order
    m_collectors["Commit Collector"] = new CommitCollector(this);
//    m_collectors["Krazy Collector"] = new KrazyCollector(this);

    // When last connector finishes its execution invoke collectFinished
    connect(m_collectors["Commit Collector"], SIGNAL(collectFinished()), this, SLOT(collectFinished()));
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
    m_configGroup = config();

    CommitCollector *commitCollector = qobject_cast<CommitCollector *>(m_collectors["Commit Collector"]);

    commitCollector->setCommitsRead(m_configGroup.readEntry("commitsRead", 0));
    if (commitCollector->commitsRead() == 0)
        commitCollector->setFullUpdate(true);

    // Config - General
    m_commitExtent = m_configGroup.readEntry("commitExtent", 1);
    m_synchronizationDelay = m_configGroup.readEntry("synchronizationDelay", 60);
    m_cacheContents = m_configGroup.readEntry("cacheContents", true);
    m_enableAnimations = m_configGroup.readEntry("enableAnimations", true);
    m_enableTransitionEffects = m_configGroup.readEntry("enableTransitionEffects", true);
    m_enableAutoViewChange = m_configGroup.readEntry("enableAutoViewChange", true);
    m_viewsDelay = m_configGroup.readEntry("viewsDelay", 5);

    QStringList viewNames = m_configGroup.readEntry("viewNames", QStringList());
    QList<bool> viewActives = m_configGroup.readEntry("viewActives", QList<bool>());

    m_activeViews.clear();
    int viewsCount = viewNames.count();
    for (int i = 0; i < viewsCount; ++i)
        m_activeViews.append(QPair<QString, bool>(viewNames.at(i), viewActives.at(i)));

    // Config - Projects
    QStringList projectNames = m_configGroup.readEntry("projectNames", QStringList());
    QStringList projectCommitSubjects = m_configGroup.readEntry("projectCommitSubjects", QStringList());
    QStringList projectIcons = m_configGroup.readEntry("projectIcons", QStringList());

    m_projects.clear();
    int projectsCount = projectNames.count();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.commitSubject = projectCommitSubjects.at(i);
        project.icon = projectIcons.at(i);
        m_projects[projectNames.at(i)] = project;
    }

    // Config - Top Active Projects
    QStringList topActiveProjectsViewNames = m_configGroup.readEntry("topActiveProjectsViewNames", QStringList());
    QList<bool> topActiveProjectsViewActives = m_configGroup.readEntry("topActiveProjectsViewActives", QList<bool>());

    m_topActiveProjectsViewProjects.clear();
    int topActiveProjectsViewsCount = topActiveProjectsViewNames.count();
    for (int i = 0; i < topActiveProjectsViewsCount; ++i)
        m_topActiveProjectsViewProjects[topActiveProjectsViewNames.at(i)] = topActiveProjectsViewActives.at(i);

    // Config - Top Developers
    QStringList topDevelopersViewNames = m_configGroup.readEntry("topDevelopersViewNames", QStringList());
    QList<bool> topDevelopersViewActives = m_configGroup.readEntry("topDevelopersViewActives", QList<bool>());

    m_topDevelopersViewProjects.clear();
    int topDevelopersViewsCount = topDevelopersViewNames.count();
    for (int i = 0; i < topDevelopersViewsCount; ++i)
        m_topDevelopersViewProjects[topDevelopersViewNames.at(i)] = topDevelopersViewActives.at(i);

    // Config - Commit History
    QStringList commitHistoryViewNames = m_configGroup.readEntry("commitHistoryViewNames", QStringList());
    QList<bool> commitHistoryViewActives = m_configGroup.readEntry("commitHistoryViewActives", QList<bool>());

    m_commitHistoryViewProjects.clear();
    int commitHistoryViewsCount = commitHistoryViewNames.count();
    for (int i = 0; i < commitHistoryViewsCount; ++i)
        m_commitHistoryViewProjects[commitHistoryViewNames.at(i)] = commitHistoryViewActives.at(i);

    // Main Layout
    QGraphicsWidget *container = new QGraphicsWidget(this);

    m_viewContainer = new QGraphicsWidget(container);
    m_viewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_viewContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    m_right = new Plasma::PushButton(container);
    m_right->nativeWidget()->setIcon(KIcon("go-next-view"));
    m_right->nativeWidget()->setToolTip(i18n("Go to previous view"));
    m_right->setMaximumSize(22, 22);
    connect(m_right->nativeWidget(), SIGNAL(clicked()), this, SLOT(moveViewRight()));

    m_left = new Plasma::PushButton(container);
    m_left->nativeWidget()->setIcon(KIcon("go-previous-view"));
    m_left->nativeWidget()->setToolTip(i18n("Go to next view"));
    m_left->setMaximumSize(22, 22);
    connect(m_left->nativeWidget(), SIGNAL(clicked()), this, SLOT(moveViewLeft()));

    m_progressProxy = new QGraphicsProxyWidget(container);
    QProgressBar *collectorProgress = new QProgressBar;
    connect(commitCollector, SIGNAL(progressMaximum(int)), collectorProgress, SLOT(setMaximum(int)));
    connect(commitCollector, SIGNAL(progressValue(int)), collectorProgress, SLOT(setValue(int)));
    m_progressProxy->setWidget(collectorProgress);
    m_progressProxy->hide();

    m_updateLabel = new Plasma::Label(container);
    m_updateLabel->setText("");
    m_updateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_updateLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_updateLabel->setAlignment(Qt::AlignCenter);

    m_horizontalLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_horizontalLayout->addItem(m_left);
    m_horizontalLayout->addItem(m_updateLabel);
    m_horizontalLayout->addItem(m_right);
    m_horizontalLayout->setMaximumHeight(22);

    QGraphicsLinearLayout *verticalLayout = new QGraphicsLinearLayout(Qt::Vertical);
    verticalLayout->addItem(m_viewContainer);
    verticalLayout->addItem(m_horizontalLayout);
    container->setLayout(verticalLayout);
    container->setGeometry(contentsRect());

    // Timers
    m_viewTransitionTimer = new QTimer(this);
    m_viewTransitionTimer->setInterval(m_viewsDelay * 1000);
    connect(m_viewTransitionTimer, SIGNAL(timeout()), this, SLOT(moveViewLeft()));

    m_synchronizationTimer = new QTimer(this);
    m_synchronizationTimer->setInterval(m_synchronizationDelay * 1000);
    connect(m_synchronizationTimer, SIGNAL(timeout()), this, SLOT(runCollectors()));

    // Creating view providers
    m_viewProviders["Top Active Projects"] = new TopActiveProjectsView(m_topActiveProjectsViewProjects, m_projects, m_viewContainer->geometry(), m_viewContainer);
    m_viewProviders["Top Developers"] = new TopDevelopersView(m_topDevelopersViewProjects, m_projects, m_viewContainer->geometry(), m_viewContainer);
    m_viewProviders["Commit History"] = new CommitHistoryView(m_commitHistoryViewProjects, m_projects, m_viewContainer->geometry(), m_viewContainer);

    commitCollector->setExtent(m_commitExtent);
    runCollectors();
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    m_configGeneral = new KdeObservatoryConfigGeneral(parent);
    parent->addPage(m_configGeneral, i18n("General"), "applications-development");

    m_configProjects = new KdeObservatoryConfigProjects(parent);
    parent->addPage(m_configProjects, i18n("Projects"), "project-development");

    m_configViews = new KdeObservatoryConfigViews(parent);
    m_configViews->m_projects = m_projects;
    m_configViews->m_projectsInView["Top Active Projects"] = m_topActiveProjectsViewProjects;
    m_configViews->m_projectsInView["Top Developers"] = m_topDevelopersViewProjects;
    m_configViews->m_projectsInView["Commit History"] = m_commitHistoryViewProjects;
    m_configViews->m_projectsInView["Krazy"] = m_krazyViewProjects;
    m_configViews->on_views_currentIndexChanged("Top Active Projects");
    parent->addPage(m_configViews, i18n("Views"), "view-presentation");

    connect(m_configProjects, SIGNAL(projectAdded(const QString &, const QString &)),
            m_configViews, SLOT(projectAdded(const QString &, const QString &)));
    connect(m_configProjects, SIGNAL(projectRemoved(const QString &)),
            m_configViews, SLOT(projectRemoved(const QString &)));

    // Config - General
    m_configGeneral->commitExtent->setValue(m_commitExtent);
    m_configGeneral->synchronizationDelay->setTime(QTime(m_synchronizationDelay/3600, (m_synchronizationDelay/60)%60, m_synchronizationDelay%60));
    m_configGeneral->cacheContents->setChecked(m_cacheContents);
    m_configGeneral->enableAnimations->setChecked(m_enableAnimations);
    m_configGeneral->enableTransitionEffects->setChecked(m_enableTransitionEffects);
    m_configGeneral->enableAutoViewChange->setChecked(m_enableAutoViewChange);
    m_configGeneral->viewsDelay->setTime(QTime(m_viewsDelay/3600, (m_viewsDelay/60)%60, m_viewsDelay%60));

    int viewsCount = m_activeViews.count();
    for (int i = 0; i < viewsCount; ++i)
    {
        QListWidgetItem * item = m_configGeneral->activeViews->findItems(m_activeViews.at(i).first, Qt::MatchFixedString).at(0);
        item->setCheckState(m_activeViews.at(i).second == true ? Qt::Checked:Qt::Unchecked);
        m_configGeneral->activeViews->takeItem(m_configGeneral->activeViews->row(item));
        m_configGeneral->activeViews->insertItem(i, item);
    }

    // Config - Projects
    foreach(QString projectName, m_projects.keys())
    {
        Project project = m_projects.value(projectName);
        m_configProjects->createTableWidgetItem(projectName, project.commitSubject, project.icon);
        m_configProjects->projects->setCurrentCell(0, 0);
    }

    m_configProjects->projects->setCurrentItem(m_configProjects->projects->item(0, 0));
    m_configProjects->projects->resizeColumnsToContents();
    m_configProjects->projects->horizontalHeader()->setStretchLastSection(true);

    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void KdeObservatory::configAccepted()
{
    prepareUpdateViews();

    CommitCollector *commitCollector = qobject_cast<CommitCollector *>(m_collectors["Commit Collector"]);

    bool commitExtentChanged = false;
    if (m_configGeneral->commitExtent->value() != m_commitExtent)
        if (m_configGeneral->commitExtent->value() > m_commitExtent)
            commitCollector->setFullUpdate(true);
        else
            commitExtentChanged = true;

    // General properties
    m_configGroup.writeEntry("commitExtent", m_commitExtent = m_configGeneral->commitExtent->value());
    QTime synchronizationDelay = m_configGeneral->synchronizationDelay->time();
    m_configGroup.writeEntry("synchronizationDelay", m_synchronizationDelay = synchronizationDelay.second() + synchronizationDelay.minute()*60 + synchronizationDelay.hour()*3600);
    m_configGroup.writeEntry("cacheContents", m_cacheContents = (m_configGeneral->cacheContents->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableAnimations", m_enableAnimations = (m_configGeneral->enableAnimations->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableTransitionEffects", m_enableTransitionEffects = (m_configGeneral->enableTransitionEffects->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableAutoViewChange", m_enableAutoViewChange = (m_configGeneral->enableAutoViewChange->checkState() == Qt::Checked) ? true:false);
    QTime viewsDelay = m_configGeneral->viewsDelay->time();
    m_configGroup.writeEntry("viewsDelay", m_viewsDelay = viewsDelay.second() + viewsDelay.minute()*60 + viewsDelay.hour()*3600);

    m_activeViews.clear();
    QStringList viewNames;
    QList<bool> viewActives;

    for (int i = 0; i < m_configGeneral->activeViews->count(); ++i)
    {
        QListWidgetItem *item = m_configGeneral->activeViews->item(i);
        QString viewName = item->text();
        bool viewActive = (item->checkState() == Qt::Checked) ? true:false;
        m_activeViews << QPair<QString, bool>(viewName, viewActive);
        viewNames << viewName;
        viewActives << viewActive;
    }
    m_configGroup.writeEntry("viewNames", viewNames);
    m_configGroup.writeEntry("viewActives", viewActives);

    commitCollector->setExtent(m_commitExtent);
    m_viewTransitionTimer->setInterval(m_viewsDelay * 1000);
    m_synchronizationTimer->setInterval(m_synchronizationDelay * 1000);

    // Projects properties
    QStringList projectNames;
    QStringList projectCommitSubjects;
    QStringList projectIcons;

    m_projects.clear();

    int projectsCount = m_configProjects->projects->rowCount();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.commitSubject = m_configProjects->projects->item(i, 1)->text();
        project.icon = m_configProjects->projects->item(i, 0)->data(Qt::UserRole).value<QString>();
        m_projects[m_configProjects->projects->item(i, 0)->text()] = project;
        projectNames << m_configProjects->projects->item(i, 0)->text();
        projectCommitSubjects << project.commitSubject;
        projectIcons << project.icon;
    }

    m_configGroup.writeEntry("projectNames", projectNames);
    m_configGroup.writeEntry("projectCommitSubjects", projectCommitSubjects);
    m_configGroup.writeEntry("projectIcons", projectIcons);

    m_configViews->on_views_currentIndexChanged("Top Active Projects");
    m_topActiveProjectsViewProjects = m_configViews->m_projectsInView["Top Active Projects"];
    m_topDevelopersViewProjects = m_configViews->m_projectsInView["Top Developers"];
    m_commitHistoryViewProjects = m_configViews->m_projectsInView["Commit History"];
    m_krazyViewProjects = m_configViews->m_projectsInView["Krazy"];

    m_configGroup.writeEntry("topActiveProjectsViewNames", m_topActiveProjectsViewProjects.keys());
    m_configGroup.writeEntry("topActiveProjectsViewActives", m_topActiveProjectsViewProjects.values());

    m_configGroup.writeEntry("topDevelopersViewNames", m_topDevelopersViewProjects.keys());
    m_configGroup.writeEntry("topDevelopersViewActives", m_topDevelopersViewProjects.values());

    m_configGroup.writeEntry("commitHistoryViewNames", m_commitHistoryViewProjects.keys());
    m_configGroup.writeEntry("commitHistoryViewActives", m_commitHistoryViewProjects.values());

    m_configGroup.writeEntry("krazyViewNames", m_krazyViewProjects.keys());
    m_configGroup.writeEntry("krazyViewActives", m_krazyViewProjects.values());

    emit configNeedsSaving();

    if (commitCollector->fullUpdate() || commitExtentChanged)
        runCollectors();
    else
        updateViews();
}

void KdeObservatory::collectFinished()
{
    prepareUpdateViews();

    setBusy(false);
    m_updateLabel->setText(i18n("Last update: ") + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));
    m_progressProxy->hide();
    m_horizontalLayout->removeItem(m_progressProxy);
    m_horizontalLayout->insertItem(1, m_updateLabel);
    m_updateLabel->show();
    m_right->setEnabled(true);
    m_left->setEnabled(true);

    updateViews();

    m_configGroup.writeEntry("commitsRead", (qobject_cast<CommitCollector *>(m_collectors["Commit Collector"]))->commitsRead());
    m_synchronizationTimer->start();
}

void KdeObservatory::moveViewRight()
{
    switchViews(-1);
}

void KdeObservatory::moveViewLeft()
{
    switchViews(1);
}

void KdeObservatory::switchViews(int delta)
{
    int previousView = m_currentView;
    int newView = m_currentView + delta;
    m_currentView = (newView >= 0) ? newView % m_views.count():m_views.count() + newView;

    if (m_enableTransitionEffects)
    {
        QGraphicsWidget *previousViewWidget = m_views.at(previousView);
        QGraphicsWidget *currentViewWidget = m_views.at(m_currentView);
        currentViewWidget->setPos(currentViewWidget->geometry().width(), 0);
        currentViewWidget->show();

        m_transitionTimer = new QTimeLine(500);
        m_transitionTimer->setFrameRange(0, 1);
        m_transitionTimer->setCurveShape(QTimeLine::EaseOutCurve);

        QGraphicsItemAnimation *animationPrevious = new QGraphicsItemAnimation;
        animationPrevious->setItem(previousViewWidget);
        animationPrevious->setTimeLine(m_transitionTimer);
        animationPrevious->setPosAt(0, QPointF(0, 0));
        animationPrevious->setPosAt(1, -delta*QPointF(previousViewWidget->geometry().width(), 0));

        QGraphicsItemAnimation *animationNew = new QGraphicsItemAnimation;
        animationNew->setItem(currentViewWidget);
        animationNew->setTimeLine(m_transitionTimer);
        animationNew->setPosAt(0, delta*QPointF(currentViewWidget->geometry().width(), 0));
        animationNew->setPosAt(1, QPointF(0, 0));

        m_transitionTimer->start();
    }
    else
    {
        m_views.at(previousView)->hide();
        m_views.at(m_currentView)->setPos(0, 0);
        m_views.at(m_currentView)->show();
    }
}

void KdeObservatory::runCollectors()
{
    m_right->setEnabled(false);
    m_left->setEnabled(false);

    setBusy(true);
    m_updateLabel->hide();
    m_horizontalLayout->removeItem(m_updateLabel);
    m_horizontalLayout->insertItem(1, m_progressProxy);
    m_progressProxy->show();

    foreach (ICollector *collector, m_collectors)
        collector->run();
}

void KdeObservatory::prepareUpdateViews()
{
    m_viewTransitionTimer->stop();
    m_synchronizationTimer->stop();
    if (m_transitionTimer)
        m_transitionTimer->stop();

    foreach(QGraphicsWidget *widget, m_views)
        widget->hide();
}

void KdeObservatory::updateViews()
{
    m_views.clear();
    int count = m_activeViews.count();
    for (int i = 0; i < count; ++i)
    {
        const QPair<QString, bool> &pair = m_activeViews.at(i);
        const QString &view = pair.first;
        if (pair.second && m_viewProviders[view])
        {
            m_viewProviders[view]->updateViews();
            m_views.append(m_viewProviders[view]->views());
        }
    }

    if (m_views.count() > 0)
    {
        m_currentView = m_views.count()-1;
        moveViewLeft();
        if (m_enableAutoViewChange)
            m_viewTransitionTimer->start();
    }
    m_synchronizationTimer->start();
}

#include "kdeobservatory.moc"
