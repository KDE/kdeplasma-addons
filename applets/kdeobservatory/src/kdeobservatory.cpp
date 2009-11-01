#include "kdeobservatory.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
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
: Plasma::Applet(parent, args), m_projects(new QStandardItemModel(this))
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

    CommitCollector *c = new CommitCollector(this);
    //c->run();
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
    m_projects->clear();
    m_projects->setHorizontalHeaderLabels(QStringList() << "Project" << "Commit Subject");

    for (int i = 0; i < projectsCount; ++i)
    {
        QStandardItem *nameItem = new QStandardItem(KIcon(projectIcons.at(i)), projectNames.at(i));
        nameItem->setData(projectIcons.at(i), Qt::UserRole);
        QStandardItem *commitSubjectItem = new QStandardItem(projectCommitSubjects.at(i));
        m_projects->appendRow(QList<QStandardItem *>() << nameItem << commitSubjectItem);
    }
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

    m_configProjects->projects->setModel(m_projects);
    m_configProjects->projects->setCurrentIndex(m_projects->index(0, 0));

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void KdeObservatory::configAccepted()
{
    QStringList projectNames;
    QStringList projectCommitSubjects;
    QStringList projectIcons;

    int projectsCount = m_projects->rowCount(m_projects->invisibleRootItem()->index());
    for (int i = 0; i < projectsCount; ++i)
    {
        projectNames << m_projects->item(i, 0)->text();
        projectCommitSubjects << m_projects->item(i, 1)->text();
        projectIcons << m_projects->item(i, 0)->data(Qt::UserRole).value<QString>();
    }

    m_configGroup.writeEntry("projectNames", projectNames);
    m_configGroup.writeEntry("projectCommitSubjects", projectCommitSubjects);
    m_configGroup.writeEntry("projectIcons", projectIcons);

    emit configNeedsSaving();
}

#include "kdeobservatory.moc"
