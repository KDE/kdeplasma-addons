/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "weatherconfigsearch.h"
#include <KMessageBox>
#include <KInputDialog>

WeatherConfigSearch::WeatherConfigSearch(QWidget *parent)
    : KDialog(parent)
    , m_dataengine(0)
{
    setupUi(mainWidget());
    setButtons(KDialog::User1 | KDialog::Cancel);
    setButtonIcon(KDialog::User1, KIcon("system-search"));
    setButtonText(KDialog::User1, i18n("&Search..."));
    setDefaultButton(KDialog::User1);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(searchPressed()));
    connect(&m_validator, SIGNAL(finished(const QString&)),
            this, SLOT(finished(const QString&)));
    connect(cityLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(textChanged(const QString&)));
    textChanged(QString());
}

WeatherConfigSearch::~WeatherConfigSearch()
{
}

void WeatherConfigSearch::textChanged(const QString& txt)
{
    enableButton(KDialog::User1, !txt.isEmpty());
}

void WeatherConfigSearch::searchPressed()
{
    QString plugin = providerComboBox->itemData(providerComboBox->currentIndex()).toString();
    QString city = cityLineEdit->text();
    m_validator.validate(plugin, city);
}

void WeatherConfigSearch::finished(const QString &source)
{
    if (!source.isEmpty()) {
        m_source = source;
        accept();
    }
}

QString WeatherConfigSearch::nameForPlugin(const QString& plugin)
{
    return providerComboBox->itemText(providerComboBox->findData(plugin));
}

void WeatherConfigSearch::setDataEngine(Plasma::DataEngine* dataengine)
{
    m_dataengine = dataengine;
    m_validator.setDataEngine(dataengine);

    providerComboBox->clear();
    if (m_dataengine) {
        const QVariantList plugins = m_dataengine->query("ions").values();
        foreach (const QVariant& plugin, plugins) {
            const QStringList pluginInfo = plugin.toString().split('|');
            providerComboBox->addItem(pluginInfo[0], pluginInfo[1]);
        }
        providerComboBox->setCurrentIndex(providerComboBox->findData("bbcukmet"));
    }
}

void WeatherConfigSearch::setSource(const QString& source)
{
    m_source = source;
    const QStringList list = source.split('|', QString::SkipEmptyParts);
    int listCount = list.count();
    if (listCount > 0) {
        providerComboBox->setCurrentIndex(providerComboBox->findData(list[0]));
    }
    if (listCount > 2) {
        cityLineEdit->setText(list[2]);
    }
    cityLineEdit->selectAll();
    cityLineEdit->setFocus(Qt::OtherFocusReason);
}

QString WeatherConfigSearch::source() const
{
    return m_source;
}

#include "weatherconfigsearch.moc"
