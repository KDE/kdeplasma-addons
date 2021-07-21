/*
    SPDX-FileCopyrightText: 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "spellcheck_config.h"

#include <QGridLayout>
#include <QProcess>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>

// For the macro
#include <KRunner/AbstractRunner>

SpellCheckConfigForm::SpellCheckConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

SpellCheckConfig::SpellCheckConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    m_ui = new SpellCheckConfigForm(this);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->m_requireTriggerWord, &QCheckBox::stateChanged, this, &SpellCheckConfig::markAsChanged);
    connect(m_ui->m_requireTriggerWord, &QCheckBox::stateChanged, this, &SpellCheckConfig::toggleTriggerWord);
    connect(m_ui->m_triggerWord, &QLineEdit::textChanged, this, &SpellCheckConfig::markAsChanged);
    connect(m_ui->m_openKcmButton, &QPushButton::clicked, this, &SpellCheckConfig::openKcm);

    m_ui->m_openKcmButton->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
}

SpellCheckConfig::~SpellCheckConfig()
{
}

void SpellCheckConfig::toggleTriggerWord(int state)
{
    m_ui->m_triggerWord->setEnabled(state == Qt::Checked);
}

void SpellCheckConfig::openKcm()
{
    QProcess::startDetached(QStringLiteral("kcmshell5"), {QStringLiteral("spellchecking")});
}

void SpellCheckConfig::load()
{
    KCModule::load();

    // FIXME: This shouldn't be hardcoded!
    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    const KConfigGroup grp = cfg->group("Runners").group("Spell Checker");

    const bool requireTrigger = grp.readEntry("requireTriggerWord", true);
    const QString trigger = grp.readEntry("trigger", i18n("spell"));

    if (!requireTrigger) {
        m_ui->m_triggerWord->setEnabled(false);
    }

    m_ui->m_requireTriggerWord->setCheckState((requireTrigger) ? Qt::Checked : Qt::Unchecked);
    m_ui->m_triggerWord->setText(trigger);

    Q_EMIT changed(false);
}

void SpellCheckConfig::save()
{
    // FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group("Spell Checker");

    bool requireTrigger = m_ui->m_requireTriggerWord->checkState() == Qt::Checked;
    if (requireTrigger) {
        grp.writeEntry("trigger", m_ui->m_triggerWord->text());
    }
    grp.writeEntry("requireTriggerWord", requireTrigger);
    grp.sync();

    Q_EMIT changed(false);
}

void SpellCheckConfig::defaults()
{
    m_ui->m_requireTriggerWord->setCheckState(Qt::Checked);
    m_ui->m_triggerWord->setText(i18n("spell"));
    Q_EMIT changed(true);
}

K_PLUGIN_FACTORY(SpellCheckConfigFactory, registerPlugin<SpellCheckConfig>();)

#include "spellcheck_config.moc"
