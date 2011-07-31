/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotPartConfig.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QInputDialog>

#include <Lancelot/Models/Serializator>
#include <Lancelot/Models/AvailableModels>
#include <Lancelot/PopupList>

#include <KDebug>

void LancelotPartConfig::setupUi(QWidget * widget)
{
    Ui::LancelotPartConfigBase::setupUi(widget);

    popup = NULL;

    qbgIcon = new QButtonGroup(widget);
    qbgIcon->addButton(radioIconActivationClick);
    qbgIcon->addButton(radioIconActivationHover);

    qbgContents = new QButtonGroup(widget);
    qbgContents->addButton(radioContentsActivationClick);
    qbgContents->addButton(radioContentsActivationExtender);

    qbgContentsExtenderPosition = new QButtonGroup(widget);
    qbgContentsExtenderPosition->addButton(radioContentsExtenderPositionLeft);
    qbgContentsExtenderPosition->addButton(radioContentsExtenderPositionRight);

    buttonContentsRemove = new QToolButton(listModels);
    buttonContentsRemove->setIcon(KIcon("list-remove"));
    buttonContentsRemove->setToolTip(i18n("Remove"));
    buttonContentsRemove->hide();

    QObject::connect(
        listModels, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(listModelsItemClicked(QListWidgetItem*)));
    QObject::connect(
        listModels, SIGNAL(itemSelectionChanged()),
        this, SLOT(listModelsItemSelectionChanged()));
    QObject::connect(
        buttonContentsRemove, SIGNAL(clicked()),
        this, SLOT(buttonContentsRemoveClicked()));
    connect(buttonContentsRemove, SIGNAL(clicked()) , this, SIGNAL(contentsChanged()));
}

bool LancelotPartConfig::iconClickActivation() const
{
    return radioIconActivationClick->isChecked();
}

bool LancelotPartConfig::contentsClickActivation() const
{
    return radioContentsActivationClick->isChecked();
}

Lancelot::ExtenderPosition LancelotPartConfig::contentsExtenderPosition() const
{
    return ((radioContentsExtenderPositionLeft->isChecked()) ?
            Lancelot::LeftExtender : Lancelot::RightExtender);
}

QString LancelotPartConfig::icon() const
{
    return iconChooser->icon();
}

void LancelotPartConfig::setIconClickActivation(bool value)
{
    if (value) {
        radioIconActivationClick->click();
    } else {
        radioIconActivationHover->click();
    }
}

void LancelotPartConfig::setContentsClickActivation(bool value)
{
    if (value) {
        radioContentsActivationClick->click();
    } else {
        radioContentsActivationExtender->click();
    }
}

void LancelotPartConfig::setContentsExtenderPosition(Lancelot::ExtenderPosition position)
{
    if (position == Lancelot::LeftExtender) {
        radioContentsExtenderPositionLeft->click();
    } else {
        radioContentsExtenderPositionRight->click();
    }
}

void LancelotPartConfig::setShowSearchBox(bool value)
{
    checkShowSearchBox->setChecked(value);
}

bool LancelotPartConfig::showSearchBox() const
{
    return checkShowSearchBox->isChecked();
}

void LancelotPartConfig::setIcon(const QString & icon)
{
    iconChooser->setIcon(icon);
}

void LancelotPartConfig::setIcon(const QIcon & icon)
{
    iconChooser->setIcon(icon);
}

void LancelotPartConfig::setPartData(const QString & data)
{
    listModels->addItem("Add...");
    QListWidgetItem * itemAdd = listModels->item(listModels->count() - 1);
    itemAdd->setData(Qt::DecorationRole, KIcon("list-add"));
    itemAdd->setData(Qt::SizeHintRole, QSize(0, 32));

    foreach (const QString & itemData, data.split('\n')) {
        addItem(itemData);
    }
}

void LancelotPartConfig::addItem(const QString & itemData)
{
    if (itemData.isEmpty()) return;

    int index = listModels->count() - 1;
    listModels->insertItem(index, QString::null);
    setItemData(listModels->item(index) , itemData);

    listModels->clearSelection();
}

void LancelotPartConfig::setItemData(
    QListWidgetItem * item, const QString & itemData)
{
    if (!item || itemData.isEmpty()) return;

    QMap < QString, QString > dataMap =
        Lancelot::Models::Serializator::deserialize(itemData);

    item->setData(Qt::DisplayRole,
        //dataMap["model"]);
        Lancelot::Models::AvailableModels::self()->titleForModel(dataMap["model"]));

    item->setData(Qt::UserRole, itemData);
    item->setData(Qt::SizeHintRole, QSize(0, 48));
    item->setData(Qt::DecorationRole, KIcon("plasmaapplet-shelf"));

}

QString LancelotPartConfig::partData() const
{
    QString result;

    for (int i = 0; i < listModels->count(); i++) {
        if (!result.isEmpty()) {
            result += '\n';
        }

        QListWidgetItem * item = listModels->item(i);
        result += item->data(Qt::UserRole).toString();

    }

    return result;
}

void LancelotPartConfig::buttonContentsAddClicked()
{
    if (!popup) {
        popup = new Lancelot::PopupList();
        connect(popup, SIGNAL(activated(int)),
            this, SLOT(buttonContentsAddItemSelected(int)));
	connect(popup, SIGNAL(activated(int)), this, SIGNAL(contentsChanged()));
        popup->setModel(Lancelot::Models::AvailableModels::self());
    }

    popup->exec(QCursor::pos());
}

void LancelotPartConfig::buttonContentsRemoveClicked()
{
    foreach (QListWidgetItem * item, listModels->selectedItems()) {
        if (item != listModels->item(listModels->count() - 1)) {
            listModels->takeItem(listModels->row(item));
        }
    }

    listModels->clearSelection();
}

void LancelotPartConfig::buttonContentsAddItemSelected(int index)
{
    kDebug() << index;
    QString data = Lancelot::Models::AvailableModels::self()->
            serializedDataForItem(index);
    kDebug() << data;

    if (!data.isEmpty()) {
        addItem(data);
        popup->hide();
    }
}

void LancelotPartConfig::listModelsItemClicked(QListWidgetItem * item)
{
    if (item == listModels->item(listModels->count() - 1)) {
        buttonContentsAddClicked();
    }
}

void LancelotPartConfig::listModelsItemSelectionChanged()
{
    QListWidgetItem * selectedItem = 0;

    if (listModels->selectedItems().count()) {
        selectedItem = listModels->selectedItems()[0];

        if (selectedItem == listModels->item(listModels->count() - 1)) {
            selectedItem = NULL;
        }
    }

    if (selectedItem) {
        QRect rect = listModels->visualItemRect(selectedItem);
        rect.setLeft(rect.right() - 32);
        rect.setHeight(32);
        buttonContentsRemove->setGeometry(rect);
        buttonContentsRemove->setVisible(true);
    } else {
        buttonContentsRemove->setVisible(false);
    }
}

