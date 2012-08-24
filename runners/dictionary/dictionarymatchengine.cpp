/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionarymatchengine.h"
#include <Plasma/AbstractRunner>
#include <QThread>
#include <QMetaMethod>

DictionaryMatchEngine::DictionaryMatchEngine(Plasma::DataEngine *dictionaryEngine, QObject *parent)
	: QObject(parent),
	  m_dictionaryEngine(dictionaryEngine)
{
	/* We have to connect source in two different places, due to the difference in
	 * how the connection is made based on data availability. There are two cases,
	 * and this extra connection handles the second case. */
	connect(m_dictionaryEngine, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));
}

/* This function should be called from a different thread. */
QString DictionaryMatchEngine::lookupWord(const QString &word)
{
	if (!m_dictionaryEngine) {
		kDebug() << "Could not find dictionary data engine.";
		return QString();
	}
	if (thread() == QThread::currentThread()) {
		kDebug() << "DictionaryMatchEngine::lookupWord is only meant to be called from non-primary threads.";
		return QString();
	}

	ThreadData data;

	m_wordLock.lockForWrite();
	m_lockers.insert(word, &data);
	m_wordLock.unlock();

	/* We lock it in this thread. Then we try to lock it again, which we cannot do, until the other thread
	 * unlocks it for us first. We time-out after 30 seconds. */
	data.mutex.lock();
	QMetaObject::invokeMethod(this, "sourceAdded", Qt::QueuedConnection, Q_ARG(const QString&, QLatin1Char(':') + word));
	if (!data.mutex.tryLock(30 * 1000))
		kDebug() << "The dictionary data engine timed out.";

	m_wordLock.lockForWrite();
	m_lockers.remove(word, &data);
	m_wordLock.unlock();

	return data.definition;
}

void DictionaryMatchEngine::sourceAdded(const QString &source)
{
	m_dictionaryEngine->connectSource(source, this);
}

void DictionaryMatchEngine::dataUpdated(const QString &source, const Plasma::DataEngine::Data &result)
{
	if (!result.contains(QLatin1String("text")))
		return;

	QString definition(result[QLatin1String("text")].toString());

	m_wordLock.lockForRead();
	foreach (ThreadData *data, m_lockers.values(source)) {
		/* Because of QString's CoW semantics, we don't have to worry about
		 * the overhead of assigning this to every item. */
		data->definition = definition;
		data->mutex.unlock();
	}
	m_wordLock.unlock();
}

#include "dictionarymatchengine.moc"
