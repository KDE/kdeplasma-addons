/*
 *   Copyright (C) 2007 Ryan Bitanga <ephebiphobic@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <KGenericFactory>
#include <sonnet/speller.h>

#include <plasma/abstractrunner.h>

/**
 * This checks the spelling of query
 */
class SpellCheckRunner : public Plasma::AbstractRunner
{
Q_OBJECT

public:
    SpellCheckRunner(QObject* parent, const QVariantList &args);
    ~SpellCheckRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);

    void reloadConfiguration();

private:
    QString m_triggerWord;
    bool m_requireTriggerWord;
    Sonnet::Speller m_speller;
};

K_EXPORT_PLASMA_RUNNER(spellcheckrunner, SpellCheckRunner)

#endif
