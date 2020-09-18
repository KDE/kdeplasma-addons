/* Copyright 2010  Anton Kreuzkamp <akreuzkamp@web.de>
 * Copyright 2020  Alexander Lohnau <alexander.lohnau@gmx.de>
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

#include "charrunner_config.h"
#include "config_keys.h"

// KF
#include <KRunner/AbstractRunner>
#include <KSharedConfig>
#include <KMessageWidget>
#include <KLocalizedString>

CharacterRunnerConfigForm::CharacterRunnerConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

CharacterRunnerConfig::CharacterRunnerConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    m_ui = new CharacterRunnerConfigForm(this);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->edit_trigger, &QLineEdit::textChanged, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->addItem, &QPushButton::clicked, this, &CharacterRunnerConfig::addItem);
    connect(m_ui->addItem, &QPushButton::clicked, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->deleteItem, &QPushButton::clicked, this, &CharacterRunnerConfig::deleteItem);
    connect(m_ui->deleteItem, &QPushButton::clicked, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->list, &QTreeWidget::itemSelectionChanged, this, &CharacterRunnerConfig::validateDeleteButton);
    connect(m_ui->edit_alias, &QLineEdit::textChanged, this, &CharacterRunnerConfig::validateAddButton);
    connect(m_ui->edit_hex, &QLineEdit::textChanged, this, &CharacterRunnerConfig::validateAddButton);
}

void CharacterRunnerConfig::load()
{
    KCModule::load();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group("CharacterRunner");

    m_ui->edit_trigger->setText(grp.readEntry(CONFIG_TRIGGERWORD, DEFAULT_TRIGGERWORD.toString()));
    const auto aliasList = grp.readEntry(CONFIG_ALIASES, QStringList());
    const auto codeList = grp.readEntry(CONFIG_CODES, QStringList());
    if (aliasList.size() == codeList.size()) {
        for (int i = 0; i < aliasList.size(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->list, 2);
            item->setText(0, aliasList[i]);
            item->setText(1, codeList[i]);
            m_ui->list->addTopLevelItem(item);
        }
    } else {
        const auto msg = new KMessageWidget(i18nc("Message that config is corrupted",
            "Config entries for alias list and code list have different sizes, ignoring all."), this);
        m_ui->verticalLayout->insertWidget(0, msg);
    }
}

void CharacterRunnerConfig::save()
{
    KCModule::save();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group("CharacterRunner");

    grp.writeEntry(CONFIG_TRIGGERWORD, m_ui->edit_trigger->text().isEmpty()
                                       ? DEFAULT_TRIGGERWORD.toString() : m_ui->edit_trigger->text());

    QList<QString> aliasList;
    QList<QString> codeList;
    for (int i = 0; i < m_ui->list->topLevelItemCount(); ++i) {
        aliasList.append(m_ui->list->topLevelItem(i)->text(0));
        codeList.append(m_ui->list->topLevelItem(i)->text(1));
    }
    grp.writeEntry(CONFIG_ALIASES, aliasList);
    grp.writeEntry(CONFIG_CODES, codeList);
    grp.sync();
}

void CharacterRunnerConfig::defaults()
{
    KCModule::defaults();

    m_ui->edit_trigger->setText(DEFAULT_TRIGGERWORD.toString());
    m_ui->list->clear();

    emit markAsChanged();
}

void CharacterRunnerConfig::addItem()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->list, 2);
    item->setText(0, m_ui->edit_alias->text());
    item->setText(1, m_ui->edit_hex->text());
    m_ui->list->addTopLevelItem(item);
    m_ui->edit_alias->clear();
    m_ui->edit_hex->clear();
}

void CharacterRunnerConfig::deleteItem()
{
    m_ui->list->takeTopLevelItem(m_ui->list->indexOfTopLevelItem(m_ui->list->currentItem()));
}

void CharacterRunnerConfig::validateAddButton()
{
    m_ui->addItem->setDisabled(m_ui->edit_alias->text().isEmpty() || m_ui->edit_hex->text().isEmpty());
}

void CharacterRunnerConfig::validateDeleteButton()
{
    m_ui->deleteItem->setDisabled(!m_ui->list->selectedItems().count());
}

K_PLUGIN_FACTORY(CharacterRunnerConfigFactory,
                 registerPlugin<CharacterRunnerConfig>(QStringLiteral("kcm_krunner_charrunner"));)

#include "charrunner_config.moc"
