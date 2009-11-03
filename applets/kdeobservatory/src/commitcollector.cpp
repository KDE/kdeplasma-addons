#include "commitcollector.h"

#include <QUrl>
#include <QDate>
#include <QDebug>
#include <QRegExp>
#include <QStandardItemModel>

CommitCollector::CommitCollector(const QList<KdeObservatory::Project> &projects, QObject *parent)
: ICollector(parent), m_extent(7), m_header("POST", "/"), m_projects(projects)
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
    QDate now = QDate::currentDate();
    m_stopCollectingDay = now.addDays(-(m_extent+1)).toString("yyyyMMdd").toLongLong();

    m_archiveName = now.toString("yyyyMM");
    qDebug() << "Requesting: " << "l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4" << ". Stop: " << m_stopCollectingDay;
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

const QMap<QString, int> &CommitCollector::resultMap() const
{
    return m_resultMap;
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
            emit collectFinished();
            return;
        }

        qDebug() << regExp.cap(1).trimmed() << "-" << regExp.cap(2).trimmed() << "-" << regExp.cap(3).trimmed() << "-" << regExp.cap(4).trimmed();

        foreach (KdeObservatory::Project project, m_projects)
        {
            QString commitSubject = project.commitSubject;
            if (path.startsWith(commitSubject))
            {
                if (m_resultMap.contains(commitSubject))
                    m_resultMap[commitSubject]++;
                else
                    m_resultMap[commitSubject] = 1;
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

    qDebug() << m_resultMap;
    qDebug() << "Requesting: " << "l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4";
    request(m_header, QString("l=kde-commits&r=" + QString::number(m_page) + "&b=" + m_archiveName + "&w=4").toUtf8());
}
