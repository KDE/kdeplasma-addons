#include "servicejobwrapper.h"

#include <attica/atticabasejob.h>
#include <attica/metadata.h>


using namespace Attica;


ServiceJobWrapper::ServiceJobWrapper(Attica::BaseJob* job, const QString& destination, const QString& operation, const QMap< QString, QVariant >& parameters, QObject* parent)
    : ServiceJob(destination, operation, parameters, parent), m_job(job)
{
    connect(m_job, SIGNAL(finished(Attica::BaseJob*)), SLOT(finished(Attica::BaseJob*)));
}


void ServiceJobWrapper::start()
{
    m_job->start();
}

void ServiceJobWrapper::finished(BaseJob* job)
{
    if (job->metadata().statusCode() != 100) {
        setError(job->metadata().statusCode());
    }
    setResult(QVariant());
}


#include "servicejobwrapper.moc"
