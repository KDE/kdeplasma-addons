#include "kdeobservatoryconfigcommithistory.h"

KdeObservatoryConfigCommitHistory::KdeObservatoryConfigCommitHistory(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
}

KdeObservatoryConfigCommitHistory::~KdeObservatoryConfigCommitHistory()
{
}

void KdeObservatoryConfigCommitHistory::createListWidgetItem(const QString &projectName, const QString &icon, bool active)
{
    QListWidgetItem *item = new QListWidgetItem(projectName, projectsInCommitHistoryView);
    item->setCheckState(active ? Qt::Checked:Qt::Unchecked);
    item->setIcon(KIcon(icon));
}

void KdeObservatoryConfigCommitHistory::projectAdded(const QString &projectName, const QString &icon)
{
    createListWidgetItem(projectName, icon, true);
}

void KdeObservatoryConfigCommitHistory::projectRemoved(const QString &projectName)
{
    projectsInCommitHistoryView->takeItem(projectsInCommitHistoryView->row(projectsInCommitHistoryView->findItems(projectName, Qt::MatchFixedString)[0]));
}
