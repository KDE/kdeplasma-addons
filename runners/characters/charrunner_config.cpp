/* Copyright 2010  Anton Kreuzkamp <akreuzkamp@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

//Project-Includes
#include "charrunner_config.h"
//KDE-Includes
#include <plasma/abstractrunner.h>

K_EXPORT_RUNNER_CONFIG(CharacterRunner, CharacterRunnerConfig)

CharacterRunnerConfigForm::CharacterRunnerConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

CharacterRunnerConfig::CharacterRunnerConfig(QWidget* parent, const QVariantList& args) :
        KCModule(ConfigFactory::componentData(), parent, args)
{
    m_ui = new CharacterRunnerConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->edit_trigger, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->addItem, SIGNAL(clicked()), this, SLOT(addItem()));
    connect(m_ui->deleteItem, SIGNAL(clicked()), this, SLOT(deleteItem()));
}

void CharacterRunnerConfig::addItem() //add Item to the list-view widget
{
  QTreeWidgetItem* item = new QTreeWidgetItem(m_ui->list, 2);
  item->setText(0, m_ui->edit_alias->text());
  item->setText(1, m_ui->edit_hex->text());
  m_ui->list->addTopLevelItem(item);
  m_ui->edit_alias->clear();
  m_ui->edit_hex->clear();

  emit changed(true);
}

void CharacterRunnerConfig::deleteItem() //remove Item to the list-view widget
{
  m_ui->list->takeTopLevelItem(m_ui->list->indexOfTopLevelItem(m_ui->list->currentItem()));

  emit changed(true);
}

void CharacterRunnerConfig::load()
{
    KCModule::load();

    //create config-object
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String( "krunnerrc" ));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "CharacterRunner");

    m_ui->edit_trigger->setText(grp.readEntry(CONFIG_TRIGGERWORD, "#")); //read out triggerword and put into the trigger-lineEdit
    for(int i=0; i<grp.readEntry(CONFIG_ALIASES, QList<QString>()).size(); i++) //read out aliaslist and add Items to the list-view widget
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(m_ui->list, 2);
      item->setText(0, grp.readEntry(CONFIG_ALIASES, QList<QString>())[i]);
      item->setText(1, grp.readEntry(CONFIG_CODES, QList<QString>())[i]);
      m_ui->list->addTopLevelItem(item);
    }
    emit changed(false);
}

void CharacterRunnerConfig::save()
{
    KCModule::save();

    //create config-object
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String( "krunnerrc" ));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "CharacterRunner");

    grp.writeEntry(CONFIG_TRIGGERWORD,m_ui->edit_trigger->text()); //write content from the triggerword-line Edit into the config

    //Write the content of the List into the config
    QList<QString> aliaslist;
    QList<QString> codelist;
    for(int i=0; i < m_ui->list->topLevelItemCount(); i++)
    {
      QString blub = m_ui->list->topLevelItem(i)->text(0);
      aliaslist.append(blub);
      codelist.append(m_ui->list->topLevelItem(i)->text(1));
    }
    grp.writeEntry(CONFIG_ALIASES, aliaslist);
    grp.writeEntry(CONFIG_CODES, codelist);

    emit changed(false);
}

void CharacterRunnerConfig::defaults()
{
    KCModule::defaults();

    m_ui->edit_trigger->setText(QLatin1String( "#" )); //set the content of the triggerword-lineEdit to default '#'
    for(int i=0; i<m_ui->list->topLevelItemCount(); i++) //remove every item from the alias-list
    {
      m_ui->list->takeTopLevelItem(i);
    }

    emit changed(true);
}
