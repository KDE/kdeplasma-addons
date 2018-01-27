/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNER_H
#define DICTIONARYRUNNER_H

#include <KRunner/AbstractRunner>
#include "dictionarymatchengine.h"

class DictionaryRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    DictionaryRunner(QObject *parent, const QVariantList &args);
    void match(Plasma::RunnerContext &context) Q_DECL_OVERRIDE;
    void reloadConfiguration() Q_DECL_OVERRIDE;

private:
    QString m_triggerWord;
    DictionaryMatchEngine *m_engine;

protected Q_SLOTS:
    void init() Q_DECL_OVERRIDE;

};

#endif
