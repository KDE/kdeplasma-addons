#include "kdeobservatoryconfigviews.h"

KdeObservatoryConfigViews::KdeObservatoryConfigViews(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    setupUi(this);
    m_lastView = views->currentText();
}

KdeObservatoryConfigViews::~KdeObservatoryConfigViews()
{
}

void KdeObservatoryConfigViews::projectAdded(const QString &projectName, const QString &icon)
{
    KdeObservatory::Project project;
    project.icon = icon;
    m_projects[projectName] = project;
    QMapIterator<QString, ViewProjects> i(m_projectsInView);
    while (i.hasNext())
    {
        i.next();
        QString view = i.key();
        m_projectsInView[view][projectName] = true;
    }
    on_views_currentIndexChanged(m_lastView);
}

void KdeObservatoryConfigViews::projectRemoved(const QString &projectName)
{
    m_projects.remove(projectName);
    QMapIterator<QString, ViewProjects> i(m_projectsInView);
    while (i.hasNext())
    {
        i.next();
        QString view = i.key();
        m_projectsInView[view].remove(projectName);
    }
}

void KdeObservatoryConfigViews::on_views_currentIndexChanged(QString view)
{
    int counter = projectsInView->count();
    for (int i = 0; i < counter; ++i)
    {
        QListWidgetItem *item = projectsInView->item(i);
        m_projectsInView[m_lastView][item->text()] = item->checkState() == Qt::Checked ? true:false;
    }
    projectsInView->clear();
    QHashIterator<QString, bool> i(m_projectsInView[view]);
    while (i.hasNext())
    {
        i.next();
        QString project = i.key();
        QListWidgetItem *item = new QListWidgetItem(project, projectsInView);
        item->setCheckState(i.value() ? Qt::Checked:Qt::Unchecked);
        item->setIcon(KIcon(m_projects[project].icon));
    }
    m_lastView = view;
}
