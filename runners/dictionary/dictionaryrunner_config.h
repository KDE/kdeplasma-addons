/*
 *   Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNERCONFIG_H
#define DICTIONARYRUNNERCONFIG_H


#include <KCModule>
class QLineEdit;

static const char CONFIG_TRIGGERWORD[] = "triggerWord";

class DictionaryRunnerConfig : public KCModule
{
	Q_OBJECT

public:
	DictionaryRunnerConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());

public slots:
	void save();
	void load();
	void defaults();

private:
	QLineEdit *m_triggerWord;
};
#endif
