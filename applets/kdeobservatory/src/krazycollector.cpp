#include "krazycollector.h"

#include <QUrl>
#include <QRegExp>

#include "kdeobservatorydatabase.h"

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
    KdeObservatoryDatabase::self()->truncateKrazyErrors();
    if (m_projects.count() == 0)
        emit collectFinished();
    else
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

    QRegExp regExp1("<li><b><u>(.*)</u></b><ol>");
    QRegExp regExp2("<li><span class=\"toolmsg\">(.*)<b>");
    QRegExp regExp3("<li><a href=\"[^<>]*" + m_idProjectMap[id] + "(.*)\">.*</a>:\\s*(.*)\\s*</li>");
    regExp1.setMinimal(true);
    regExp2.setMinimal(true);
    regExp3.setMinimal(true);

    int pos = 0, pos1, pos2, pos3;
    QString fileType;
    QString testName;
    pos1 = regExp1.indexIn(source, pos);
    pos2 = regExp2.indexIn(source, pos);
    pos3 = regExp3.indexIn(source, pos);
    while (pos1 != -1 || pos2 != -1 || pos3 != -1)
    {
        pos = pos1;
        if (pos2 != -1 && pos2 < pos)
            pos = pos2;
        if (pos3 != -1 && pos3 < pos)
            pos = pos3;
        if (pos == pos1)
        {
            fileType = regExp1.cap(1);
            pos += regExp1.matchedLength();
        }
        else if (pos == pos2)
        {
            testName = regExp2.cap(1);
            pos += regExp2.matchedLength();
        }
        else if (pos == pos3)
        {
            KdeObservatoryDatabase::self()->addKrazyError(m_idProjectMap[id], fileType, testName, regExp3.cap(1), regExp3.cap(2));
            pos += regExp3.matchedLength();
        }
        pos1 = regExp1.indexIn(source, pos);
        pos2 = regExp2.indexIn(source, pos);
        pos3 = regExp3.indexIn(source, pos);
    }

    ++m_projectsCollected;
    if (m_projectsCollected == m_projects.count())
        emit collectFinished();
}

void KrazyCollector::collectProject(const QString &project)
{
    int id = get(QUrl::toPercentEncoding("/krazy/reports/" + m_projects[project].krazyReport + "/index.html", "!$&'()*+,;=:@/"));
    m_idProjectMap[id] = m_projects[project].commitSubject.split("/").last();
}
