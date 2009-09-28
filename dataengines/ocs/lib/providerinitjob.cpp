#include "providerinitjob.h"

#include <QTimer>

#include <KUrl>


using namespace Attica;


class ProviderInitJob::Private {
    public:
        QString m_id;
        Provider m_provider;
        Private(const QString& id) : m_id(id)
        {
        }
};


ProviderInitJob::ProviderInitJob(const QString& id, QObject* parent)
    : KJob(parent), d(new Private(id))
{
}


ProviderInitJob::~ProviderInitJob()
{
    delete d;
}


void ProviderInitJob::start()
{
    QTimer::singleShot(0, this, SLOT(doWork()));
}


void ProviderInitJob::doWork()
{
    if (d->m_id == "opendesktop") {
        d->m_provider = Provider(d->m_id, KUrl("https://api.opendesktop.org/v1/"), "OpenDesktop.org");
    }
    emitResult();
}


Provider ProviderInitJob::provider() const
{
    return d->m_provider;
}


#include "providerinitjob.moc"
