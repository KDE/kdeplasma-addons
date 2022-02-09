/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "potd.h"

#include <QDate>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThreadPool>
#include <QTimer>

#include <KPluginLoader>
#include <KPluginMetaData>
#include <Plasma/DataContainer>

#include "cachedprovider.h"

namespace
{
namespace DataKeys
{
auto image = []() noexcept {
    return QStringLiteral("Image");
};
auto url = []() noexcept {
    return QStringLiteral("Url");
};
}
}

PotdEngine::PotdEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // set polling to every 5 minutes
    setMinimumPollingInterval(5 * 60 * 1000);
    m_checkDatesTimer = new QTimer(this); // change picture after 24 hours
    connect(m_checkDatesTimer, &QTimer::timeout, this, &PotdEngine::checkDayChanged);
    // FIXME: would be nice to stop and start this timer ONLY as needed, e.g. only when there are
    // time insensitive sources to serve; still, this is better than how i found it, checking
    // every 2 seconds (!)
    m_checkDatesTimer->setInterval(10 * 60 * 1000); // check every 10 minutes
    m_checkDatesTimer->start();

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("potd"));

    for (const auto &metadata : plugins) {
        QString provider = metadata.value(QLatin1String("X-KDE-PlasmaPoTDProvider-Identifier"));
        if (provider.isEmpty()) {
            continue;
        }
        mFactories.insert(provider, metadata);
        setData(QLatin1String("Providers"), provider, metadata.name());
    }
}

PotdEngine::~PotdEngine()
{
}

bool PotdEngine::updateSourceEvent(const QString &identifier)
{
    return updateSource(identifier, false);
}

bool PotdEngine::updateSource(const QString &identifier, bool loadCachedAlways)
{
    // check whether it is cached already...
    if (CachedProvider::isCached(identifier, loadCachedAlways)) {
        QVariantList args;
        args << QLatin1String("String") << identifier;

        CachedProvider *provider = new CachedProvider(identifier, this);
        connect(provider, &PotdProvider::finished, this, &PotdEngine::finished);
        connect(provider, &PotdProvider::error, this, &PotdEngine::error);

        m_canDiscardCache = loadCachedAlways;
        if (!loadCachedAlways) {
            return true;
        }
    }

    const QStringList parts = identifier.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    if (parts.empty()) {
        qDebug() << "invalid identifier";
        return false;
    }
    const QString providerName = parts[0];
    if (!mFactories.contains(providerName)) {
        qDebug() << "invalid provider: " << parts[0];
        return false;
    }

    QVariantList args;

    for (int i = 0; i < parts.count(); i++) {
        args << parts[i];
    }

    auto factory = KPluginLoader(mFactories[providerName].fileName()).factory();
    PotdProvider *provider = nullptr;
    if (factory) {
        provider = factory->create<PotdProvider>(this, args);
    }
    if (provider) {
        connect(provider, &PotdProvider::finished, this, &PotdEngine::finished);
        connect(provider, &PotdProvider::error, this, &PotdEngine::error);
        return true;
    }

    return false;
}

bool PotdEngine::sourceRequestEvent(const QString &identifier)
{
    if (updateSource(identifier, true)) {
        setData(identifier, DataKeys::image(), QImage());
        return true;
    }

    return false;
}

void PotdEngine::finished(PotdProvider *provider)
{
    if (m_canDiscardCache && qobject_cast<CachedProvider *>(provider)) {
        Plasma::DataContainer *source = containerForSource(provider->identifier());
        if (source && !source->data().value(DataKeys::image()).value<QImage>().isNull()) {
            provider->deleteLater();
            return;
        }
    }

    QImage img(provider->image());
    // store in cache if it's not the response of a CachedProvider
    if (qobject_cast<CachedProvider *>(provider) == nullptr && !img.isNull()) {
        SaveImageThread *thread = new SaveImageThread(provider->identifier(), img);
        connect(thread, &SaveImageThread::done, this, &PotdEngine::cachingFinished);
        QThreadPool::globalInstance()->start(thread);
    } else {
        setData(provider->identifier(), DataKeys::image(), img);
        setData(provider->identifier(), DataKeys::url(), CachedProvider::identifierToPath(provider->identifier()));
    }

    provider->deleteLater();
}

void PotdEngine::cachingFinished(const QString &source, const QString &path, const QImage &img)
{
    setData(source, DataKeys::image(), img);
    setData(source, DataKeys::url(), path);
}

void PotdEngine::error(PotdProvider *provider)
{
    provider->disconnect(this);
    provider->deleteLater();
}

void PotdEngine::checkDayChanged()
{
    SourceDict dict = containerDict();
    QHashIterator<QString, Plasma::DataContainer *> it(dict);
    QRegularExpression re(QLatin1String(":\\d{4}-\\d{2}-\\d{2}"));

    while (it.hasNext()) {
        it.next();

        if (it.key() == QLatin1String("Providers")) {
            continue;
        }

        // Check if the identifier contains ISO date string, like 2019-01-09.
        // If so, don't update the picture. Otherwise, update the picture.
        if (!re.match(it.key()).hasMatch()) {
            const QString path = CachedProvider::identifierToPath(it.key());
            if (!QFile::exists(path)) {
                updateSourceEvent(it.key());
            } else {
                QFileInfo info(path);
                if (info.lastModified().daysTo(QDateTime::currentDateTime()) >= 1) {
                    updateSourceEvent(it.key());
                }
            }
        }
    }
}

K_PLUGIN_CLASS_WITH_JSON(PotdEngine, "plasma-dataengine-potd.json")

#include "potd.moc"
