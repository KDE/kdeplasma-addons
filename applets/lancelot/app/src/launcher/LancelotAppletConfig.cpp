/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "LancelotAppletConfig.h"
#include "lancelot_interface.h"

#include <KIconDialog>

void LancelotAppletConfig::setupUi(QWidget * widget)
{
    Ui::LancelotAppletConfigBase::setupUi(widget);

    qbgIcons = new QButtonGroup(widget);
    qbgIcons->addButton(radioShowCategories);
    qbgIcons->addButton(radioShowMenuIconOnly);

    qbgMenuActivation = new QButtonGroup(widget);
    qbgMenuActivation->addButton(radioActivationHover);
    qbgMenuActivation->addButton(radioActivationClick);

    connect(radioShowCategories, SIGNAL(clicked()),
            this, SLOT(updateCard()));
    connect(radioShowMenuIconOnly, SIGNAL(clicked()),
            this, SLOT(updateCard()));
    connect(listIcons, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(iconItemClicked()));

    org::kde::lancelot::App lancelot(
            "org.kde.lancelot", "/Lancelot",
            QDBusConnection::sessionBus()
    );

    QDBusReply<QStringList> replyIDs   = lancelot.sectionIDs();
    QDBusReply<QStringList> replyNames = lancelot.sectionNames();
    QDBusReply<QStringList> replyIcons = lancelot.sectionIcons();

    // showing icons
    QListWidgetItem * item;
    item = new QListWidgetItem(
            KIcon("lancelot"), i18n("Lancelot"), listIcons);
    icons["lancelot"] = item;
    item = new QListWidgetItem(
            KIcon("kde"), i18n("KDE Logo"), listIcons);
    icons["kde"] = item;
    item = new QListWidgetItem(
            KIcon("start-here"), i18n("Start here"), listIcons);
    icons["start-here"] = item;
    item = new QListWidgetItem(
            KIcon("unknown"), i18n("Custom"), listIcons);
    icons["custom"] = item;

    // showing categs
    if (!replyIDs.isValid() || !replyNames.isValid() || !replyIcons.isValid()) {
        // Error connecting to Lancelot via d-bus
        // setFailedToLaunch(true);
        return;
    }

    for (int i = 0; i < replyIDs.value().size(); i++) {
        QListWidgetItem * item = new QListWidgetItem(
                KIcon(replyIcons.value().at(i)), replyNames.value().at(i));
        item->setData(Qt::UserRole, replyIDs.value().at(i));
        categories[replyIDs.value().at(i)] = item;
        listSections->addItem(item);
        item->setSelected(true);
    }
}

void LancelotAppletConfig::iconItemClicked()
{
    if (!icons.contains("custom")) {
        return;
    }

    if (icons["custom"]->isSelected()) {
        QString newCustomIcon = KIconDialog::getIcon();
        if (!newCustomIcon.isEmpty()) {
            customIcon = newCustomIcon;
            icons["custom"]->setIcon(KIcon(customIcon));
        }
    }
}

bool LancelotAppletConfig::showCategory(const QString & id) const
{
    return categories.value(id)->isSelected();
}

bool LancelotAppletConfig::showCategories() const
{
    return radioShowCategories->isChecked();
}

bool LancelotAppletConfig::clickActivation() const
{
    return radioActivationClick->isChecked();
}

QStringList LancelotAppletConfig::showingCategories(bool value) const
{
    QStringList result;
    foreach (QListWidgetItem * item, categories) {
        if (item->isSelected() == value) {
            result << item->data(Qt::UserRole).toString();
        }
    }
    return result;
}

QString LancelotAppletConfig::icon() const
{
    foreach (QString id, icons.keys()) {
        QListWidgetItem * item = icons[id];
        if (item->isSelected()) {
            if (id == "custom") {
                return customIcon;
            }
            return id;
        }
    }
    return "lancelot";
}

void LancelotAppletConfig::setShowAllCategories(bool value)
{
    foreach (QListWidgetItem * item, categories) {
        item->setSelected(value);
    }
}

void LancelotAppletConfig::setShowingCategories(QStringList ids, bool value)
{
    foreach (QString id, categories.keys()) {
        QListWidgetItem * item = categories[id];
        item->setSelected((ids.contains(id)) ? value : (!value));
    }
}

void LancelotAppletConfig::setShowCategory(const QString & id, bool value)
{
    categories.value(id)->setSelected(value);
}

void LancelotAppletConfig::setShowCategories(bool value)
{
    if (value) {
        radioShowCategories->click();
    } else {
        radioShowMenuIconOnly->click();
    }
}

void LancelotAppletConfig::setClickActivation(bool value)
{
    if (value) {
        radioActivationClick->click();
    } else {
        radioActivationHover->click();
    }
}

void LancelotAppletConfig::setIcon(const QString & icon)
{
    bool found = false;
    foreach (QString id, icons.keys()) {
        QListWidgetItem * item = icons[id];
        item->setSelected(id == icon);
        if (id == icon) {
            found = true;
        }
    }

    if (found || !icons.contains("custom")) {
        return;
    }

    customIcon = icon;
    icons["custom"]->setSelected(true);
    icons["custom"]->setIcon(KIcon(customIcon));
}

void LancelotAppletConfig::updateCard()
{
    if (radioShowCategories->isChecked()) {
        stackedAppletButtons->setCurrentWidget(pageCategoriesChoose);
    } else {
        stackedAppletButtons->setCurrentWidget(pageAppletIconsChoose);
    }
}


