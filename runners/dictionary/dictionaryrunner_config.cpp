/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner_config.h"
#include <Plasma/AbstractRunner>
#include <QFormLayout>
#include <QLineEdit>

K_EXPORT_RUNNER_CONFIG(dictionary, DictionaryRunnerConfig)

DictionaryRunnerConfig::DictionaryRunnerConfig(QWidget* parent, const QVariantList& args) :
	KCModule(ConfigFactory::componentData(), parent, args)
{
	QFormLayout *layout = new QFormLayout;
	m_triggerWord = new QLineEdit;
	layout->addRow(i18n("Trigger Word"), m_triggerWord);
	setLayout(layout);
	connect(m_triggerWord, SIGNAL(textChanged(QString)), this, SLOT(changed()));
	load();
}

void DictionaryRunnerConfig::load()
{
	KCModule::load();
	KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
	KConfigGroup grp = cfg->group("Runners");
	grp = KConfigGroup(&grp, "Dictionary");
	m_triggerWord->setText(grp.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define")));
	emit changed(false);
}

void DictionaryRunnerConfig::save()
{
	KCModule::save();
	KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
	KConfigGroup grp = cfg->group("Runners");
	grp = KConfigGroup(&grp, "Dictionary");
	grp.writeEntry(CONFIG_TRIGGERWORD, m_triggerWord->text());
	emit changed(false);
}

void DictionaryRunnerConfig::defaults()
{
	KCModule::defaults();
	m_triggerWord->setText(i18nc("Trigger word before word to define", "define"));
	emit changed(true);
}

#include "dictionaryrunner_config.moc"
