/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner_config.h"
#include <KLocalizedString>
#include <KPluginFactory>
#include <KRunner/AbstractRunner>
#include <KSharedConfig>
#include <QFormLayout>
#include <QLineEdit>

K_PLUGIN_FACTORY(DictionaryRunnerConfigFactory, registerPlugin<DictionaryRunnerConfig>(QStringLiteral("kcm_krunner_dictionary"));)

DictionaryRunnerConfig::DictionaryRunnerConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QFormLayout *layout = new QFormLayout;
    m_triggerWord = new QLineEdit;
    layout->addRow(i18nc("@label:textbox", "Trigger word:"), m_triggerWord);
    setLayout(layout);
    connect(m_triggerWord, &QLineEdit::textChanged, this, &DictionaryRunnerConfig::markAsChanged);
    load();
}

void DictionaryRunnerConfig::load()
{
    KCModule::load();
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "Dictionary");
    m_triggerWord->setText(grp.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define")));
    Q_EMIT changed(false);
}

void DictionaryRunnerConfig::save()
{
    KCModule::save();
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "Dictionary");
    grp.writeEntry(CONFIG_TRIGGERWORD, m_triggerWord->text());
    grp.sync();
    Q_EMIT changed(false);
}

void DictionaryRunnerConfig::defaults()
{
    KCModule::defaults();
    m_triggerWord->setText(i18nc("Trigger word before word to define", "define"));
    Q_EMIT changed(true);
}

#include "dictionaryrunner_config.moc"
