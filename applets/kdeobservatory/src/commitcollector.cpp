#include "commitcollector.h"

#include <QUrl>
#include <QDate>
#include <QRegExp>
#include <QStandardItemModel>

#include "kdeobservatorydatabase.h"

CommitCollector::CommitCollector(QObject *parent)
: ICollector(parent),
  m_fullUpdate(true),
  m_commitsRead(0),
  m_extent(1),
  m_header("POST", "/"),
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
    if (extent > m_extent)
        m_fullUpdate = true;
    m_extent = extent;
}

int CommitCollector::extent() const
{
    return m_extent;
}

void CommitCollector::run()
{
    m_page = 1;

    QDate now = QDate::currentDate();
    QString stopDate = now.addDays(-m_extent-1).toString("yyyyMMdd");
    m_stopCollectingDay = stopDate.toLongLong();

    KdeObservatoryDatabase::deleteOldCommits(stopDate);
    if (m_fullUpdate)
    {
        KdeObservatoryDatabase::truncateCommits();
        m_fullUpdate = false;
        m_commitsRead = 0;
    }

    m_archiveName = now.toString("yyyyMM");
    request(m_header, QString("l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4").toUtf8());
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
        QString developer = regExp.cap(4).trimmed();
        long long date = regExp.cap(2).trimmed().remove("-").toLongLong();

        if (date <= m_stopCollectingDay)
        {
            emit collectFinished();
            return;
        }

        KdeObservatoryDatabase::self()->addCommit(regExp.cap(2).trimmed(), path, developer);
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
