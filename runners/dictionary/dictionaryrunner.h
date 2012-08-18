/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNER_H
#define DICTIONARYRUNNER_H

#include <Plasma/AbstractRunner>
#include "dictionarymatchengine.h"

class DictionaryRunner : public Plasma::AbstractRunner
{
	Q_OBJECT

public:
	DictionaryRunner(QObject *parent, const QVariantList &args);
	void match(Plasma::RunnerContext &context);
	void reloadConfiguration();

private:
	QString m_triggerWord;
	DictionaryMatchEngine *m_engine;

protected slots:
	void init();

};

#endif
