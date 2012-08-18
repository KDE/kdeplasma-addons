/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYMATCHENGINE_H
#define DICTIONARYMATCHENGINE_H

#include <Plasma/DataEngine>
#include <QHash>
#include <QReadWriteLock>
#include <QMutex>
#include <QMultiMap>

namespace Plasma
{
class RunnerContext;
}
class DictionaryMatchEngine : public QObject
{
	Q_OBJECT

public:
	DictionaryMatchEngine(Plasma::DataEngine *dictionaryEngine, QObject *parent = 0);
	QString lookupWord(const QString &word);

private:
	struct ThreadData {
		QMutex mutex;
		QString definition;
	};
	QMultiMap <QString, ThreadData*> m_lockers;
	QReadWriteLock m_wordLock;
	Plasma::DataEngine *m_dictionaryEngine;

private slots:
	void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
	void sourceAdded(const QString &source);

};

#endif
