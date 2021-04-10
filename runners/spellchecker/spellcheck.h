/*
 *   SPDX-FileCopyrightText: 2007 Ryan Bitanga <ephebiphobic@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <sonnet/speller.h>

#include <KRunner/AbstractRunner>
#include <QMutex>
#include <QSharedPointer>

/**
 * This checks the spelling of query
 */
class SpellCheckRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    SpellCheckRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~SpellCheckRunner() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;

    void reloadConfiguration() override;

protected Q_SLOTS:
    void init() override;
    QMimeData *mimeDataForMatch(const Plasma::QueryMatch &match) override;

    void loadData();
    void destroydata();

private:
    QString findLang(const QStringList &terms);

    QString m_triggerWord;
    QMap<QString, QString> m_languages; // key=language name, value=language code
    bool m_requireTriggerWord;
    QMap<QString, QSharedPointer<Sonnet::Speller>> m_spellers; // spellers
    QMutex m_spellLock; // Lock held when constructing a new speller
};

#endif
