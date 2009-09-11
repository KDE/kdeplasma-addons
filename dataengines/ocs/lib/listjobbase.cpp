#include "listjobbase.h"

#include <QtCore/QTimer>

#include <KIO/Job>


using namespace Attica;

void ListJobBase::setUrl(const KUrl& url)
{
    m_url = url;
}


void ListJobBase::start()
{
    QTimer::singleShot(0, this, SLOT(doWork()));
}


void ListJobBase::doWork()
{
    KJob* job = KIO::get(m_url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), SLOT(slotJobData(KIO::Job*, const QByteArray&)));
    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobResult(KJob*)));
}


void ListJobBase::slotJobData(KIO::Job* job, const QByteArray& data)
{
    Q_UNUSED(job);

    m_data.append(data);
}


void ListJobBase::slotJobResult(KJob* job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
    
        emitResult();
    } else {
        parse(QString::fromUtf8(m_data.data()));

        emitResult();
    }
}


#include "listjobbase.moc"
