#include "commitcollector.h"

#include <QUrl>
#include <QDate>
#include <QRegExp>
#include <QStandardItemModel>

bool pairLessThan(const QPair<QString, int> &p1, const QPair<QString, int> &p2)
{
     return p1.second < p2.second;
}

CommitCollector::CommitCollector(const QMap<QString, KdeObservatory::Project> &projects, QObject *parent)
: ICollector(parent),
  m_extent(1),
  m_header("POST", "/"),
  m_projects(projects)
{
    m_connectId = setHost("lists.kde.org", QHttp::ConnectionModeHttp, 0);
    m_header.setValue("Host", "lists.kde.org");
    m_header.setContentType("application/x-www-form-urlencoded");
}

CommitCollector::~CommitCollector()
{
}

void CommitCollector::run()
{
    m_page = 1;

    m_tempCommits.clear();
    m_resultingCommits.clear();
    m_tempCommiters.clear();
    m_resultingCommiters.clear();

    QDate now = QDate::currentDate();
    m_stopCollectingDay = now.addDays(-(m_extent+1)).toString("yyyyMMdd").toLongLong();

    m_archiveName = now.toString("yyyyMM");
    request(m_header, QString("l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4").toUtf8());
}

void CommitCollector::setExtent (int extent)
{
    m_extent = extent;
}

int CommitCollector::extent() const
{
    return m_extent;
}

const QList< QPair <QString, int> > &CommitCollector::resultingCommits() const
{
    return m_resultingCommits;
}

const QHash<QString, QList< QPair<QString, int> > > &CommitCollector::resultingCommiters() const
{
    return m_resultingCommiters;
}

void CommitCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    QRegExp regExp("(\\d+)\\.\\s*(\\d{4}-\\d{2}-\\d{2})\\s*<a href=\".*\">(.*)</a>\\s*<a.*</a>(.*)(\n|\r|\f)");
    regExp.setMinimal(true);
    QString source = readAll();

    int pos = 0;
    while ((pos = regExp.indexIn(source, pos)) != -1)
    {
        QString path = regExp.cap(3).trimmed();
        QString commiter = regExp.cap(4).trimmed();
        long long date = regExp.cap(2).trimmed().remove("-").toLongLong();

        if (date <= m_stopCollectingDay)
        {
            createSortedLists();
            emit collectFinished();
            return;
        }

        foreach (QString projectName, m_projects.keys())
        {
            KdeObservatory::Project project = m_projects.value(projectName);
            QRegExp commitSubject(project.commitSubject);
            if (commitSubject.indexIn(path, 0) != -1)
            {
                if (m_tempCommits.contains(projectName))
                    m_tempCommits[projectName]++;
                else
                    m_tempCommits[projectName] = 1;
                if (m_tempCommiters.contains(projectName) && m_tempCommiters[projectName].contains(commiter))
                    m_tempCommiters[projectName][commiter]++;
                else
                    m_tempCommiters[projectName][commiter] = 1;
            }
        }
        pos += regExp.matchedLength();
    }
    ++m_page;
    if (!source.contains(">Next<"))
    {
        m_page = 1;
        m_archiveName = QDate::fromString(m_archiveName + "01", "yyyyMMdd").addDays(-1).toString("yyyyMM");
    }

    request(m_header, QString("l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4").toUtf8());
}

void CommitCollector::createSortedLists()
{
    QHashIterator<QString, int> i1(m_tempCommits);
    while (i1.hasNext())
    {
        i1.next();
        m_resultingCommits.append(QPair<QString, int>(i1.key(), i1.value()));
    }
    qSort(m_resultingCommits.begin(), m_resultingCommits.end(), pairLessThan);

    QHashIterator< QString, QHash<QString, int> > i2(m_tempCommiters);
    while (i2.hasNext())
    {
        i2.next();
        QList< QPair<QString, int> > list;
        QHashIterator<QString, int> i3(i2.value());
        while (i3.hasNext())
        {
            i3.next();
            list.append(QPair<QString, int>(i3.key(), i3.value()));
        }
        qSort(list.begin(), list.end(), pairLessThan);
        m_resultingCommiters[i2.key()] = list;
    }
}
