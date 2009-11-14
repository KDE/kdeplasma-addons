#include "krazycollector.h"

#include <QUrl>

#include "kdeobservatorydatabase.h"

#include <QDebug>

KrazyCollector::KrazyCollector(const QMap<QString, KdeObservatory::Project> &projects, QObject *parent)
: ICollector(parent),
  m_projects(projects)
{
    m_connectId = setHost("www.englishbreakfastnetwork.org", QHttp::ConnectionModeHttp, 0);
}

KrazyCollector::~KrazyCollector()
{
}

void KrazyCollector::run()
{
    m_projectsCollected = 0;
    m_idCommitSubjectMap.clear();
    foreach(const QString &project, m_projects.keys())
        collectProject(project);
}

void KrazyCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    QString source = readAll();

    if (source.contains("<h1>Not Found</h1>"))
    {
        QString url = m_idCommitSubjectMap[id];
        url.right(url.length()-url.lastIndexOf("/"));
        m_idCommitSubjectMap[get(url)] = url;
    }
    else
    {
        ++m_projectsCollected;
        if (m_projectsCollected == m_projects.count())
            emit collectFinished();
    }
}

void KrazyCollector::collectProject(const QString &project)
{
    QString url = QUrl::toPercentEncoding("krazy/reports/" + m_projects[project].commitSubject + "/index.html");
    m_idCommitSubjectMap[get(url)] = url;
}
