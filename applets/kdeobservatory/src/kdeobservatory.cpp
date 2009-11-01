#include "kdeobservatory.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QStandardItemModel>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KConfig>
#include <KConfigDialog>

#include "kdeobservatoryconfiggeneral.h"
#include "kdeobservatoryconfigprojects.h"
#include "ui_kdeobservatoryconfigcommitsummary.h"

#include "commitcollector.h"

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(200, 200);

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(m_view = new QGraphicsView);
    m_view->setScene(m_scene = new QGraphicsScene);
    //m_view->setBackgroundBrush(QColor(0, 0, 0));
    //m_view->setAutoFillBackground(true);
    m_scene->addEllipse(0, 0, 50, 50);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->addItem(proxy);
    setLayout(layout);
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
    m_configGroup = config();

    QStringList projectNames = m_configGroup.readEntry("projectNames", QStringList());
    QStringList projectCommitSubjects = m_configGroup.readEntry("projectCommitSubjects", QStringList());
    QStringList projectIcons = m_configGroup.readEntry("projectIcons", QStringList());

    int projectsCount = projectNames.count();
    m_projects.clear();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.name = projectNames.at(i);
        project.commitSubject = projectCommitSubjects.at(i);
        project.icon = projectIcons.at(i);
        m_projects.append(project);
    }

    CommitCollector *c = new CommitCollector(m_projects, this);
//    c->run();
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    m_configGeneral = new KdeObservatoryConfigGeneral(parent);
    parent->addPage(m_configGeneral, i18n("General"), "applications-development");

    m_configProjects = new KdeObservatoryConfigProjects(parent);
    parent->addPage(m_configProjects, i18n("Projects"), "project-development");

    QWidget *configCommitSummary = new QWidget(parent);
    Ui::KdeObservatoryConfigCommitSummary *ui_configCommitSummary = new Ui::KdeObservatoryConfigCommitSummary;
    ui_configCommitSummary->setupUi(configCommitSummary);
    parent->addPage(configCommitSummary, i18n("Commit Summary"), "svn-commit");

    int projectCount = m_projects.count();
    for (int i = 0; i < projectCount; ++i)
    {
        QTableWidgetItem *itemProject = new QTableWidgetItem(KIcon(m_projects.at(i).icon), m_projects.at(i).name);
        itemProject->setData(Qt::UserRole, m_projects.at(i).icon);
        QTableWidgetItem *itemCommitSubject = new QTableWidgetItem(m_projects.at(i).commitSubject);
        int rowCount = m_configProjects->projects->rowCount();
        m_configProjects->projects->setRowCount(rowCount+1);
        m_configProjects->projects->setItem(rowCount, 0, itemProject);
        m_configProjects->projects->setItem(rowCount, 1, itemCommitSubject);
        m_configProjects->projects->setRowHeight(rowCount, m_configProjects->projects->rowHeight(rowCount)*0.75);
        m_configProjects->projects->setCurrentItem(itemProject);
    }
    m_configProjects->projects->resizeColumnsToContents();
    m_configProjects->projects->horizontalHeader()->setStretchLastSection(true);

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void KdeObservatory::configAccepted()
{
    QStringList projectNames;
    QStringList projectCommitSubjects;
    QStringList projectIcons;

    m_projects.clear();

    int projectsCount = m_configProjects->projects->rowCount();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.name = m_configProjects->projects->item(i, 0)->text();
        project.commitSubject = m_configProjects->projects->item(i, 1)->text();
        project.icon = m_configProjects->projects->item(i, 0)->data(Qt::UserRole).value<QString>();
        m_projects.append(project);
        projectNames << project.name;
        projectCommitSubjects << project.commitSubject;
        projectIcons << project.icon;
    }

    m_configGroup.writeEntry("projectNames", projectNames);
    m_configGroup.writeEntry("projectCommitSubjects", projectCommitSubjects);
    m_configGroup.writeEntry("projectIcons", projectIcons);

    emit configNeedsSaving();
}

#include "kdeobservatory.moc"
