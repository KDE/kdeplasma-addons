#include "kdeobservatory.h"

#include <QTimer>
#include <QTimeLine>
#include <QProgressBar>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsItemAnimation>

#include <KConfig>
#include <KConfigDialog>

#include <Plasma/PushButton>

#include "kdeobservatoryconfiggeneral.h"
#include "kdeobservatoryconfigprojects.h"
#include "ui_kdeobservatoryconfigtopactiveprojects.h"

#include "commitcollector.h"
#include "topactiveprojectsview.h"
#include "topdevelopersview.h"

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args),
  m_collector (new CommitCollector(this)),
  m_currentView(0)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(300, 200);

    connect(m_collector, SIGNAL(collectFinished()), this, SLOT(collectFinished()));
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
    m_configGroup = config();

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

    // Main Layout
    QGraphicsWidget *container = new QGraphicsWidget(this);

    m_viewContainer = new QGraphicsWidget(container);
    m_viewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_viewContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    Plasma::PushButton *right = new Plasma::PushButton(container);
    Plasma::PushButton *left = new Plasma::PushButton(container);
    right->setIcon(KIcon("go-next-view"));
    left->setIcon(KIcon("go-previous-view"));
    right->nativeWidget()->setToolTip(i18n("Go to next view"));
    left->nativeWidget()->setToolTip(i18n("Go to previous view"));
    right->setMaximumSize(22, 22);
    left->setMaximumSize(22, 22);
    connect(right->nativeWidget(), SIGNAL(clicked()), this, SLOT(moveViewRight()));
    connect(left->nativeWidget(), SIGNAL(clicked()), this, SLOT(moveViewLeft()));

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(container);
    m_collectorProgress = new QProgressBar;
    connect(m_collector, SIGNAL(progressMaximum(int)), m_collectorProgress, SLOT(setMaximum(int)));
    connect(m_collector, SIGNAL(progressValue(int)), m_collectorProgress, SLOT(setValue(int)));
    proxy->setWidget(m_collectorProgress);
    proxy->hide();

    QGraphicsLinearLayout *horizontalLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    horizontalLayout->addItem(left);
    horizontalLayout->addItem(proxy);
    horizontalLayout->addItem(right);
    horizontalLayout->setMaximumHeight(22);

    QGraphicsLinearLayout *verticalLayout = new QGraphicsLinearLayout(Qt::Vertical);
    verticalLayout->addItem(m_viewContainer);
    verticalLayout->addItem(horizontalLayout);
    container->setLayout(verticalLayout);
    container->setGeometry(contentsRect());

    // Timers
    m_viewTransitionTimer = new QTimer(this);
    m_viewTransitionTimer->setInterval(m_viewsDelay * 1000);
    connect(m_viewTransitionTimer, SIGNAL(timeout()), this, SLOT(moveViewLeft()));

    m_synchronizationTimer = new QTimer(this);
    m_synchronizationTimer->setInterval(m_synchronizationDelay * 1000);
    connect(m_synchronizationTimer, SIGNAL(timeout()), this, SLOT(runCollectors()));

    runCollectors();
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    m_configGeneral = new KdeObservatoryConfigGeneral(parent);
    parent->addPage(m_configGeneral, i18n("General"), "applications-development");

    m_configProjects = new KdeObservatoryConfigProjects(parent);
    parent->addPage(m_configProjects, i18n("Projects"), "project-development");

    QWidget *configTopActiveProjects = new QWidget(parent);
    Ui::KdeObservatoryConfigTopActiveProjects *ui_configTopActiveProjects = new Ui::KdeObservatoryConfigTopActiveProjects;
    ui_configTopActiveProjects->setupUi(configTopActiveProjects);
    parent->addPage(configTopActiveProjects, i18n("Top Active Projects"), "svn-commit");

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
    }

    m_configProjects->projects->setCurrentItem(m_configProjects->projects->item(0, 0));
    m_configProjects->projects->resizeColumnsToContents();
    m_configProjects->projects->horizontalHeader()->setStretchLastSection(true);

    m_viewTransitionTimer->stop();
    m_synchronizationTimer->stop();

    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void KdeObservatory::configAccepted()
{
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

    m_collector->setExtent(m_commitExtent);
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

    emit configNeedsSaving();

    runCollectors();
}

void KdeObservatory::collectFinished()
{
    setBusy(false);
    m_collectorProgress->hide();

    TopActiveProjectsView *topActiveProjectsView = new TopActiveProjectsView(m_projects, m_viewContainer->geometry(), m_viewContainer);
    TopDevelopersView *topDevelopersView = new TopDevelopersView(m_projects, m_viewContainer->geometry(), m_viewContainer);

    m_views = topActiveProjectsView->views();
    m_views.append(topDevelopersView->views());

    m_currentView = m_views.count()-1;
    moveViewLeft();

    if (m_enableAutoViewChange)
        m_viewTransitionTimer->start();

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
    m_currentView = (newView >= 0) ? newView % m_views.count():m_views.count()+newView;

    if (m_enableTransitionEffects)
    {
        QGraphicsWidget *previousViewWidget = m_views.at(previousView);
        QGraphicsWidget *currentViewWidget = m_views.at(m_currentView);
        currentViewWidget->setPos(currentViewWidget->geometry().width(), 0);
        currentViewWidget->show();

        QTimeLine *timer = new QTimeLine(500);
        timer->setFrameRange(0, 1);
        timer->setCurveShape(QTimeLine::EaseOutCurve);

        QGraphicsItemAnimation *animationPrevious = new QGraphicsItemAnimation;
        animationPrevious->setItem(previousViewWidget);
        animationPrevious->setTimeLine(timer);
        animationPrevious->setPosAt(0, QPointF(0, 0));
        animationPrevious->setPosAt(1, -delta*QPointF(previousViewWidget->geometry().width(), 0));

        QGraphicsItemAnimation *animationNew = new QGraphicsItemAnimation;
        animationNew->setItem(currentViewWidget);
        animationNew->setTimeLine(timer);
        animationNew->setPosAt(0, delta*QPointF(currentViewWidget->geometry().width(), 0));
        animationNew->setPosAt(1, QPointF(0, 0));

        timer->start();
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
    if (m_views.at(m_currentView))
        m_views.at(m_currentView)->hide();

    setBusy(true);
    m_collectorProgress->show();
    m_collector->run();
}

#include "kdeobservatory.moc"
