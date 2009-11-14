#include "krazycollector.h"

#include <QUrl>

#include "kdeobservatorydatabase.h"

#include <QDebug>

KrazyCollector::KrazyCollector(QObject *parent)
: ICollector(parent)
{
    m_connectId = setHost("www.englishbreakfastnetwork.org", QHttp::ConnectionModeHttp, 0);
}

KrazyCollector::~KrazyCollector()
{
}

void KrazyCollector::run()
{
    get(QUrl::toPercentEncoding("krazy/reports/playground/base/plasma/applets/index.html"));
}

void KrazyCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    QString source = readAll();
    qDebug() << source;

    emit collectFinished();
}
