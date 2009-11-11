#include "kdeobservatoryconfigtopdevelopers.h"

KdeObservatoryConfigTopDevelopers::KdeObservatoryConfigTopDevelopers(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
}

KdeObservatoryConfigTopDevelopers::~KdeObservatoryConfigTopDevelopers()
{
}

void KdeObservatoryConfigTopDevelopers::createListWidgetItem(const QString &projectName, const QString &icon, bool active)
{
    QListWidgetItem *item = new QListWidgetItem(projectName, projectsInTopDevelopersView);
    item->setCheckState(active ? Qt::Checked:Qt::Unchecked);
    item->setIcon(KIcon(icon));
}

void KdeObservatoryConfigTopDevelopers::projectAdded(const QString &projectName, const QString &icon)
{
    createListWidgetItem(projectName, icon, true);
}

void KdeObservatoryConfigTopDevelopers::projectRemoved(const QString &projectName)
{
    projectsInTopDevelopersView->takeItem(projectsInTopDevelopersView->row(projectsInTopDevelopersView->findItems(projectName, Qt::MatchFixedString)[0]));
}
