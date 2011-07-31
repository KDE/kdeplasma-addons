/********************************************************************

 Copyright (C) 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "spellcheck_config.h"

#include <QGridLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KPluginLoader>

//For the macro
#include <plasma/abstractrunner.h>

K_EXPORT_RUNNER_CONFIG(spellcheck, SpellCheckConfig)

SpellCheckConfigForm::SpellCheckConfigForm(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
}

SpellCheckConfig::SpellCheckConfig(QWidget* parent, const QVariantList& args) :
        KCModule(ConfigFactory::componentData(), parent, args)
{
    m_ui = new SpellCheckConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);

    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->m_requireTriggerWord, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(m_ui->m_requireTriggerWord, SIGNAL(stateChanged(int)), this, SLOT(toggleTriggerWord(int)));
    connect(m_ui->m_triggerWord, SIGNAL(textChanged(QString)), this, SLOT(changed()));

    load();
}

SpellCheckConfig::~SpellCheckConfig()
{
}

void SpellCheckConfig::toggleTriggerWord(int state)
{
    if (state == Qt::Unchecked) {
        m_ui->m_triggerWord->setEnabled(false);
    } else {
        m_ui->m_triggerWord->setEnabled(true);
    }
}

void SpellCheckConfig::load()
{
    KCModule::load();

    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( QLatin1String( "krunnerrc" ) );
    KConfigGroup conf = cfg->group( "Runners" );
    KConfigGroup grp = KConfigGroup( &conf, "Spell Checker");

    const bool requireTrigger = grp.readEntry("requireTriggerWord", true);
    const QString trigger = grp.readEntry("trigger", i18n("spell"));

    if (!requireTrigger) {
        m_ui->m_triggerWord->setEnabled(false);
    }

    m_ui->m_requireTriggerWord->setCheckState( (requireTrigger) ? Qt::Checked : Qt::Unchecked );
    m_ui->m_triggerWord->setText( trigger );

    emit changed(false);
}

void SpellCheckConfig::save()
{
    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( QLatin1String( "krunnerrc" ) );
    KConfigGroup conf = cfg->group( "Runners" );
    KConfigGroup grp = KConfigGroup( &conf, "Spell Checker");

    bool requireTrigger = m_ui->m_requireTriggerWord->checkState() == Qt::Checked;
    if (requireTrigger) {
        grp.writeEntry( "trigger", m_ui->m_triggerWord->text() );
    }
    grp.writeEntry( "requireTriggerWord", requireTrigger );
    grp.sync();

    emit changed(false);
}

void SpellCheckConfig::defaults()
{
    m_ui->m_requireTriggerWord->setCheckState( Qt::Checked );
    m_ui->m_triggerWord->setText( i18n("spell") );
    emit changed(true);
}


#include "spellcheck_config.moc"
