/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "autopasteconfig.h"
#include "appkey.h"
#include "configdata.h"
#include <KDebug>
#include <QCheckBox>
#include <QPointer>

AutoPasteConfig::AutoPasteConfig(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    addButton->setIcon(KIcon("list-add"));
    removeButton->setIcon(KIcon("list-remove"));
    editButton->setIcon(KIcon("list-edit"));
    pasteKeyButton->setClearButtonShown(false);
    appsTreeView->setModel(&m_appModel);
    m_appModel.setHorizontalHeaderLabels(QStringList() << i18n("Application") << i18n("Paste Key"));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeClicked()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editClicked()));
    connect(autoPasteCheckBox, SIGNAL(clicked()), this, SLOT(enableWidgets()));
    connect(appsTreeView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(enableWidgets()));
    enableWidgets();
}

AutoPasteConfig::~AutoPasteConfig()
{
}

void AutoPasteConfig::setData(const ConfigData &data)
{
    pasteKeyButton->setKeySequence(data.pasteKey);
    autoPasteCheckBox->setChecked(data.autoPaste);

    foreach (const QString &key, data.specialApps.keys()) {
        QStandardItem *appItem = new QStandardItem(KIcon(key.toLower()), key);
        QStandardItem *keyItem = new QStandardItem(data.specialApps[key].toString());
        m_appModel.appendRow(QList<QStandardItem*>() << appItem << keyItem);
    }
    enableWidgets();
}

void AutoPasteConfig::getData(ConfigData *data)
{
    data->pasteKey = pasteKeyButton->keySequence();
    data->autoPaste = autoPasteCheckBox->isChecked();
    data->specialApps.clear();
    for (int i = 0; i < m_appModel.rowCount(); ++i) {
        QStandardItem *appItem = m_appModel.item(i, 0);
        QStandardItem *keyItem = m_appModel.item(i, 1);
        data->specialApps[appItem->text()] = QKeySequence::fromString(keyItem->text());
    }
}

void AutoPasteConfig::addClicked()
{
    QPointer<AppKey> dlg = new AppKey(this);
    if (dlg->exec() == QDialog::Accepted) {
        QStandardItem *appItem = new QStandardItem(KIcon(dlg->app.toLower()), dlg->app);
        QStandardItem *keyItem = new QStandardItem(dlg->pasteButton->keySequence().toString());
        m_appModel.appendRow(QList<QStandardItem*>() << appItem << keyItem);
        enableWidgets();
    }

    delete dlg;
}

void AutoPasteConfig::removeClicked()
{
    m_appModel.takeRow(appsTreeView->selectionModel()->currentIndex().row());
    enableWidgets();
}

void AutoPasteConfig::editClicked()
{
    QPointer<AppKey> dlg = new AppKey(this);
    int row = appsTreeView->selectionModel()->currentIndex().row();
    QStandardItem *appItem = m_appModel.item(row, 0);
    QStandardItem *keyItem = m_appModel.item(row, 1);
    dlg->appButton->setText(appItem->text());
    dlg->appButton->setIcon(KIcon(appItem->text().toLower()));
    dlg->pasteButton->setKeySequence(QKeySequence::fromString(keyItem->text()));
    if (dlg->exec() == QDialog::Accepted) {
        appItem->setText(dlg->app);
        appItem->setIcon(KIcon(dlg->app.toLower()));
        keyItem->setText(dlg->pasteButton->keySequence().toString());
    }

    delete dlg;
}

void AutoPasteConfig::enableWidgets()
{
    addButton->setEnabled(autoPasteCheckBox->isChecked());
    removeButton->setEnabled(autoPasteCheckBox->isChecked() &&
                             appsTreeView->selectionModel()->currentIndex().isValid());
    editButton->setEnabled(autoPasteCheckBox->isChecked() &&
                           appsTreeView->selectionModel()->currentIndex().isValid());
    pasteKeyButton->setEnabled(autoPasteCheckBox->isChecked());
    appsTreeView->setEnabled(autoPasteCheckBox->isChecked());
}

#include "autopasteconfig.moc"
