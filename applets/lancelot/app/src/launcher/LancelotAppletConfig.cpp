/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotAppletConfig.h"

void LancelotAppletConfig::setupUi(QWidget * widget)
{
    Ui::LancelotAppletConfigBase::setupUi(widget);

    iconLancelot->setPixmap((new KIcon("lancelot"))->pixmap(48));
    iconKDE->setPixmap((new KIcon("kde"))->pixmap(48));
    iconStartHere->setPixmap((new KIcon("start-here"))->pixmap(48));

    qbgIcons = new QButtonGroup(widget);
    qbgIcons->addButton(radioShowCategories);
    qbgIcons->addButton(radioShowMenuIconOnly);

    qbgChooseIcon = new QButtonGroup(widget);
    qbgChooseIcon->addButton(radioIconLancelot);
    qbgChooseIcon->addButton(radioIconKDE);
    qbgChooseIcon->addButton(radioIconStartHere);
    qbgChooseIcon->addButton(radioIconCustom);

    qbgMenuActivation = new QButtonGroup(widget);
    qbgMenuActivation->addButton(radioActivationHover);
    qbgMenuActivation->addButton(radioActivationClick);
}

bool LancelotAppletConfig::showCategories() const
{
    return radioShowCategories->isChecked();
}

bool LancelotAppletConfig::clickActivation() const
{
    return radioActivationClick->isChecked();
}

QString LancelotAppletConfig::icon() const
{
    if (radioIconLancelot->isChecked()) {
        return "lancelot";
    }
    if (radioIconKDE->isChecked()) {
        return "kde";
    }
    if (radioIconStartHere->isChecked()) {
        return "start-here";
    }
    if (radioIconCustom->isChecked()) {
        return iconCustom->icon();
    }
    return "lancelot";
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
    if (icon == "lancelot") {
        radioIconLancelot->click();
    } else if (icon == "kde") {
        radioIconKDE->click();
    } else if (icon == "start-here") {
        radioIconStartHere->click();
    } else {
        radioIconCustom->click();
        iconCustom->setIcon(icon);
    }
}
