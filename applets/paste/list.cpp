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

#include "list.h"
#include "sendkeys.h"
#include "configdata.h"
#include "pastemacroexpander.h"

#include <QTimer>
#include <QStandardItemModel>
#include <QApplication>
#include <QClipboard>

#include <KWindowSystem>
#include <KDebug>
#include <KGlobalSettings>

#include <Plasma/Delegate>
#include <Plasma/Theme>

ListForm::ListForm(QWidget *parent)
    : QWidget(parent), m_hide(false), cfg(0)
{
    setupUi(this);
    setAttribute(Qt::WA_NoSystemBackground);
    icon->setPixmap(KIcon("edit-paste").pixmap(KIconLoader::SizeSmall,
                                               KIconLoader::SizeSmall));

    Plasma::Delegate *delegate = new Plasma::Delegate;
    delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
    delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
    treeView->setItemDelegate(delegate);
    m_listModel = new QStandardItemModel(this);
    treeView->setModel(m_listModel);
    treeView->setFocusPolicy(Qt::NoFocus);

    if (KGlobalSettings::singleClick()) {
        connect(treeView, SIGNAL(clicked(const QModelIndex &)),
                this, SLOT(clicked(const QModelIndex &)));
    } else {
        connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)),
                this, SLOT(clicked(const QModelIndex &)));
    }
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    themeChanged();
}

ListForm::~ListForm()
{
}

void ListForm::setData(const ConfigData &data)
{
    m_listModel->clear();
    foreach (const QString &txt, data.snippets.keys()) {
        QStandardItem *item = new QStandardItem();
        QString tmp = data.snippets[txt][ConfigData::Text];
        item->setData(txt, Qt::DisplayRole);
        item->setData(false, SubTitleMandatoryRole);
        item->setData(tmp.replace('\n', ' '), SubTitleRole);
        item->setData(data.snippets[txt][ConfigData::Text], DataRole);
        item->setData(KIcon(data.snippets[txt][ConfigData::Icon]), Qt::DecorationRole);
        m_listModel->insertRow(0, item);
    }
    if (data.autoPaste) {
        label->setText(i18n("Text &snippets (Click to paste text):"));
    } else {
        label->setText(i18n("Text &snippets (Click to copy text to clipboard):"));
    }
    cfg = &data;
}

void ListForm::clicked(const QModelIndex &index)
{
    QList<WId> windows = KWindowSystem::stackingOrder();
    KWindowInfo info;
    static const QStringList classes =
            QStringList() << "Plasma" << "Plasma-desktop" << "Plasmoidviewer";

    // Don't paste to plasma windows
    for (int i = windows.count() - 1; i >= 0; --i) {
        info = KWindowSystem::windowInfo(windows[i], 0, NET::WM2WindowClass);
        //kDebug() << info.windowClassClass();
        if (classes.contains(info.windowClassClass())) {
            if (i > 0) {
                continue;
            }
            return;
        }
        break;
    }
    QString txt = m_listModel->data(index, DataRole).toString();
    PasteMacroExpander::instance().expandMacros(txt);
    //kDebug() << txt;
    QApplication::clipboard()->setText(txt);
    emit textCopied();
    if (m_hide) {
        hide();
    }
    // Macro expander might change windows focus so activate windows after that
    KWindowSystem::activateWindow(info.win());
    if (cfg->autoPaste) {
        if (cfg->specialApps.contains(info.windowClassClass())) {
            m_pasteKey = cfg->specialApps[info.windowClassClass()];
        } else {
            m_pasteKey = cfg->pasteKey;
        }
        QTimer::singleShot(200, this, SLOT(paste()));
    }
    treeView->selectionModel()->clear();
}

void ListForm::paste()
{
    SendKeys::self() << m_pasteKey;
}

void ListForm::themeChanged()
{
    label->setStyleSheet(QString("QLabel{color:%1;}")
            .arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name()));
    setStyleSheet(QString(".ListForm{background-color:%1;}")
            .arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor).name()));
}

void ListForm::setHideAfterClick(bool hide)
{
    m_hide = hide;
}

#include "list.moc"
