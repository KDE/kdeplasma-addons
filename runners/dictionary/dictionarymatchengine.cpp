/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionarymatchengine.h"
#include <KRunner/AbstractRunner>
#include <QThread>
#include <QMetaMethod>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QDeadlineTimer>
#endif
DictionaryMatchEngine::DictionaryMatchEngine(Plasma::DataEngine *dictionaryEngine, QObject *parent)
    : QObject(parent),
      m_dictionaryEngine(dictionaryEngine)
{
    /* We have to connect source in two different places, due to the difference in
     * how the connection is made based on data availability. There are two cases,
     * and this extra connection handles the second case. */
    Q_ASSERT(m_dictionaryEngine);
    connect(m_dictionaryEngine, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));
}

/* This function should be called from a different thread. */
QString DictionaryMatchEngine::lookupWord(const QString &word)
{
    if (!m_dictionaryEngine) {
        qDebug() << "Could not find dictionary data engine.";
        return QString();
    }
    if (thread() == QThread::currentThread()) {
        qDebug() << "DictionaryMatchEngine::lookupWord is only meant to be called from non-primary threads.";
        return QString();
    }

    ThreadData data;

    m_wordLock.lockForWrite();
    m_lockers.insert(word, &data);
    m_wordLock.unlock();

    QMetaObject::invokeMethod(this, "sourceAdded", Qt::QueuedConnection, Q_ARG(const QString&, word));
    QMutexLocker locker(&data.mutex);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    if (!data.waitCondition.wait(&data.mutex, 30 * 1000)) // Timeout after 30 seconds
#else
    if (!data.waitCondition.wait(&data.mutex, QDeadlineTimer(30 * 1000))) // Timeout after 30 seconds
#endif
        qDebug() << "The dictionary data engine timed out (word:" << word << ")";
    locker.unlock();

    QMetaObject::invokeMethod(this, "sourceRemoved", Qt::QueuedConnection, Q_ARG(const QString&, word));
    // after a timeout, if dataUpdated gets m_wordLock here, it can lock data->mutex successfully.

    m_wordLock.lockForWrite();
    m_lockers.remove(word, &data);
    m_wordLock.unlock();

    // after a timeout, if dataUpdated gets m_wordLock here, it won't see this data instance anymore.

    locker.relock();
    return data.definition;
}

void DictionaryMatchEngine::sourceAdded(const QString &source)
{
    m_dictionaryEngine->connectSource(source, this);
}

void DictionaryMatchEngine::sourceRemoved(const QString &source)
{
    m_dictionaryEngine->disconnectSource(source, this);
}

void DictionaryMatchEngine::dataUpdated(const QString &source, const Plasma::DataEngine::Data &result)
{
    if (!result.contains(QLatin1String("text")))
        return;

    QString definition(result[QLatin1String("text")].toString());

    m_wordLock.lockForRead();
    foreach (ThreadData *data, m_lockers.values(source)) {
        QMutexLocker locker(&data->mutex);
        /* Because of QString's CoW semantics, we don't have to worry about
         * the overhead of assigning this to every item. */
        data->definition = definition;
        data->waitCondition.wakeOne();
    }
    m_wordLock.unlock();
}

