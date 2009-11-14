#include "krazycollector.h"

#include <QUrl>
#include <QRegExp>

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
    QRegExp regExp3("<li><a href=\".*" + m_idProjectMap[id] + "(.*)\">.*</a>:\\s*(.*)\\s*</li>");
    regExp1.setMinimal(true);
    regExp2.setMinimal(true);
    regExp3.setMinimal(true);

    int pos = 0;
    enum State {Initial, FoundFileType, FoundTool};
    State state = Initial;
    QRegExp regExp = regExp1;
    QString fileType;
    QString testName;
    while ((pos = regExp.indexIn(source, pos)) != -1)
    {
        pos += regExp.matchedLength();
        if (state == Initial)
        {
            fileType = regExp.cap(1);
            state = FoundFileType;
            regExp = regExp2;
        }
        else if (state == FoundFileType)
        {
            testName = regExp.cap(1);
            state = FoundTool;
            regExp = regExp3;
        }
        else
            KdeObservatoryDatabase::self()->addKrazyError(m_idProjectMap[id], fileType, testName, regExp.cap(1), regExp.cap(2));
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
