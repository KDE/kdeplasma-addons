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

#include "snippetconfig.h"
#include "configdata.h"
#include "addmacro.h"
#include <KDebug>

SnippetConfig::SnippetConfig(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    addButton->setIcon(KIcon("list-add"));
    removeButton->setIcon(KIcon("list-remove"));
    addMacroButton->setIcon(KIcon("system-run"));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeClicked()));
    connect(addMacroButton, SIGNAL(clicked()), this, SLOT(addMacroClicked()));
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(enableWidgets()));
    connect(nameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(nameChanged(QString)));
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(iconButton, SIGNAL(iconChanged(QString)),
            this, SLOT(iconChanged(QString)));
    enableWidgets();
}

SnippetConfig::~SnippetConfig()
{
}

void SnippetConfig::setData(const ConfigData &data)
{
    list->clear();
    foreach (const QString &txt, data.snippets.keys()) {
        if (!txt.isEmpty() || !data.snippets[txt].isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(txt, list);
            item->setData(TextRole, data.snippets[txt][ConfigData::Text]);
            item->setData(IconNameRole, data.snippets[txt][ConfigData::Icon]);
            item->setData(Qt::DecorationRole, KIcon(data.snippets[txt][ConfigData::Icon]));
        }
    }
}

void SnippetConfig::getData(ConfigData *data)
{
    QListWidgetItem *current = list->currentItem();
    if (current) {
        current->setData(Qt::UserRole, textEdit->toPlainText());
    }
    data->snippets.clear();
    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if (!item->text().isEmpty() || !item->data(TextRole).toString().isEmpty()) {
            data->snippets[item->text()] = QStringList() <<
                    item->data(IconNameRole).toString() << item->data(TextRole).toString();
        }
    }
}

void SnippetConfig::addClicked()
{
    newItem();
    nameEdit->setFocus();
    nameEdit->selectAll();
}

void SnippetConfig::addMacroClicked()
{
    QPointer<AddMacro> dlg = new AddMacro(this);
    if (dlg->exec() == QDialog::Accepted) {
        textEdit->insertPlainText(dlg->macro());
    }
    delete dlg;
}

void SnippetConfig::removeClicked()
{
    delete list->currentItem();
}

void SnippetConfig::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous) {
        previous->setData(TextRole, textEdit->toPlainText());
        previous->setData(IconNameRole, iconButton->icon());
    }
    if (current) {
        nameEdit->setText(current->text());
        textEdit->setPlainText(current->data(TextRole).toString());
        iconButton->setIcon(current->data(IconNameRole).toString());
    } else {
        nameEdit->blockSignals(true);
        textEdit->blockSignals(true);
        nameEdit->setText("");
        textEdit->setPlainText("");
        iconButton->setIcon("");
        nameEdit->blockSignals(false);
        textEdit->blockSignals(false);
    }
}

void SnippetConfig::nameChanged(const QString& name)
{
    QListWidgetItem *current =  list->currentItem();

    if (!current) {
        current = newItem(QString(), name);
    }
    current->setText(name);
}

void SnippetConfig::textChanged()
{
    QListWidgetItem *current =  list->currentItem();

    if (!current) {
        newItem(textEdit->toPlainText());
    }
}
void SnippetConfig::enableWidgets()
{
    removeButton->setEnabled(list->selectionModel()->currentIndex().isValid());
    iconButton->setEnabled(list->selectionModel()->currentIndex().isValid());
}

void SnippetConfig::iconChanged(const QString &icon)
{
    QListWidgetItem *current =  list->currentItem();

    if (current) {
        current->setData(IconNameRole, icon);
        current->setData(Qt::DecorationRole, KIcon(icon));
    }
}

QListWidgetItem *SnippetConfig::newItem(const QString& text, const QString& name)
{
    QListWidgetItem *item = new QListWidgetItem(name, list);
    item->setData(TextRole, text);
    item->setData(IconNameRole, "edit-paste");
    item->setData(Qt::DecorationRole, KIcon("edit-paste"));
    list->setCurrentItem(item);
    QTextCursor tc = textEdit->textCursor();
    tc.setPosition(text.size());
    textEdit->setTextCursor(tc);
    return item;
}

#include "snippetconfig.moc"
