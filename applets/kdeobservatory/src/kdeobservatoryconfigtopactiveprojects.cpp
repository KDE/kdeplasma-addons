#include "kdeobservatoryconfigtopactiveprojects.h"

KdeObservatoryConfigTopActiveProjects::KdeObservatoryConfigTopActiveProjects(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
}

KdeObservatoryConfigTopActiveProjects::~KdeObservatoryConfigTopActiveProjects()
{
}

void KdeObservatoryConfigTopActiveProjects::createListWidgetItem(const QString &projectName, const QString &icon, bool active)
{
    QListWidgetItem *item = new QListWidgetItem(projectName, projectsInTopActiveProjectsView);
    item->setCheckState(active ? Qt::Checked:Qt::Unchecked);
    item->setIcon(KIcon(icon));
}

void KdeObservatoryConfigTopActiveProjects::projectAdded(const QString &projectName, const QString &icon)
{
    createListWidgetItem(projectName, icon, true);
}

void KdeObservatoryConfigTopActiveProjects::projectRemoved(const QString &projectName)
{
    projectsInTopActiveProjectsView->takeItem(projectsInTopActiveProjectsView->row(projectsInTopActiveProjectsView->findItems(projectName, Qt::MatchFixedString)[0]));
}
