/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNER_H
#define DICTIONARYRUNNER_H

#include <KRunner/AbstractRunner>
#include "dictionarymatchengine.h"

class DictionaryRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit DictionaryRunner(QObject *parent, const QVariantList &args);
    void match(Plasma::RunnerContext &context) override;
    void reloadConfiguration() override;

private:
    QString m_triggerWord;
    DictionaryMatchEngine *m_engine;

protected Q_SLOTS:
    void init() override;

};

#endif
