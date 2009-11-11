#include "commitcollector.h"

#include <QUrl>
#include <QDate>
#include <QRegExp>
#include <QStandardItemModel>

#include "kdeobservatorydatabase.h"

CommitCollector::CommitCollector(QObject *parent)
: ICollector(parent),
  m_fullUpdate(false),
  m_commitsRead(0),
  m_lastArchiveRead(""),
  m_extent(1),
  m_header("POST", "/")
{
    m_connectId = setHost("lists.kde.org", QHttp::ConnectionModeHttp, 0);
    m_header.setValue("Host", "lists.kde.org");
    m_header.setContentType("application/x-www-form-urlencoded");
}

CommitCollector::~CommitCollector()
{
}

void CommitCollector::setExtent (int extent)
{
    m_extent = extent;
}

int CommitCollector::extent() const
{
    return m_extent;
}

void CommitCollector::setCommitsRead(int commitsRead)
{
    m_commitsRead = commitsRead;
}

int CommitCollector::commitsRead() const
{
    return m_commitsRead;
}

void CommitCollector::setFullUpdate(bool fullUpdate)
{
    m_fullUpdate = fullUpdate;
}

bool CommitCollector::fullUpdate() const
{
    return m_fullUpdate;
}

void CommitCollector::run()
{
    m_page = 1;

    QDate now = QDate::currentDate();
    QString stopDate = now.addDays(-m_extent-1).toString("yyyyMMdd");
    m_stopCollectingDay = stopDate.toLongLong();
    m_lastDay = now.toString("yyyyMMdd").toLongLong();
    m_dayCounter = 0;

    KdeObservatoryDatabase::self()->deleteOldCommits(stopDate);
    if (m_fullUpdate)
    {
        KdeObservatoryDatabase::self()->truncateCommits();
        m_commitsRead = 0;
    }

    m_commits.clear();
    m_initialArchiveName = m_archiveName = now.toString("yyyyMM");
    emit progressMaximum(m_extent);
    request(m_header, QString("l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4").toUtf8());
}

void CommitCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    QString source = readAll();

    if (m_page == 1)
    {
        QRegExp regExpMsg("\\((\\d+) messages\\)");
        regExpMsg.indexIn(source, 0);
        m_commitsToBeRead = regExpMsg.cap(1).trimmed().toInt();
    }

    QRegExp regExp("(\\d+)\\.\\s*(\\d{4}-\\d{2}-\\d{2})\\s*<a href=\".*\">(.*)</a>\\s*<a.*</a>(.*)(\n|\r|\f)");
    regExp.setMinimal(true);

    int pos = 0;
    while ((pos = regExp.indexIn(source, pos)) != -1)
    {
        QString path = regExp.cap(3).trimmed();
        QString developer = regExp.cap(4).trimmed();
        long long date = regExp.cap(2).trimmed().remove("-").toLongLong();

        if (date != m_lastDay)
        {
            ++m_dayCounter;
            m_lastDay = date;
            emit progressValue(m_dayCounter);
        }

        if (date <= m_stopCollectingDay || // It reaches the whole commit extent
            (m_commitsRead == m_commitsToBeRead && // It read the delta commits ...
                   (m_lastArchiveRead.isEmpty() || m_archiveName == m_lastArchiveRead))) // ... in last read archive
        {
            m_lastArchiveRead = m_initialArchiveName;
            m_commitsRead = m_commitsToBeRead;
            while(!m_commits.isEmpty())
            {
                const Commit &commit = m_commits.pop();
                KdeObservatoryDatabase::self()->addCommit(commit.date, commit.subject, commit.developer);
            }

            m_fullUpdate = false;
            emit collectFinished();
            return;
        }

        Commit commit;
        commit.date = regExp.cap(2).trimmed();
        commit.subject = path;
        commit.developer = developer;
        m_commits.push(commit);

        ++m_commitsRead;
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
