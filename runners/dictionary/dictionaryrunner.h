/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNER_H
#define DICTIONARYRUNNER_H

#include "../../dict/dictengine.h"
#include <KRunner/AbstractRunner>

using namespace Plasma;

class DictionaryRunner : public AbstractRunner
{
    Q_OBJECT

public:
    explicit DictionaryRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    void match(RunnerContext &context) override;
    void reloadConfiguration() override;

private:
    QString m_triggerWord;
    DictEngine m_dictEngine;
};

#endif
