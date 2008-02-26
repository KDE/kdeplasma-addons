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

LancelotAppletConfig::LancelotAppletConfig()
{
    setCaption(i18n("Configure Lancelot Launcher"));

    QWidget * widget = new QWidget();
    m_ui.setupUi(widget);

    m_ui.iconLancelot->setPixmap((new KIcon("lancelot"))->pixmap(48));
    m_ui.iconKDE->setPixmap((new KIcon("kde"))->pixmap(48));
    m_ui.iconStartHere->setPixmap((new KIcon("start-here"))->pixmap(48));

    qbgIcons.addButton(m_ui.radioShowCategories);
    qbgIcons.addButton(m_ui.radioShowMenuIconOnly);

    qbgChooseIcon.addButton(m_ui.radioIconLancelot);
    qbgChooseIcon.addButton(m_ui.radioIconKDE);
    qbgChooseIcon.addButton(m_ui.radioIconStartHere);
    qbgChooseIcon.addButton(m_ui.radioIconCustom);

    qbgMenuActivation.addButton(m_ui.radioActivationHover);
    qbgMenuActivation.addButton(m_ui.radioActivationClick);

    setMainWidget(widget);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
}

LancelotAppletConfig::~LancelotAppletConfig()
{
}

bool LancelotAppletConfig::showCategories() const
{
    return m_ui.radioShowCategories->isChecked();
}

bool LancelotAppletConfig::clickActivation() const
{
    return m_ui.radioActivationClick->isChecked();
}

QString LancelotAppletConfig::icon() const
{
    if (m_ui.radioIconLancelot->isChecked()) {
        return "lancelot";
    }
    if (m_ui.radioIconKDE->isChecked()) {
        return "kde";
    }
    if (m_ui.radioIconStartHere->isChecked()) {
        return "start-here";
    }
    if (m_ui.radioIconCustom->isChecked()) {
        return m_ui.iconCustom->icon();
    }
    return "lancelot";
}

void LancelotAppletConfig::setShowCategories(bool value)
{
    if (value) {
        m_ui.radioShowCategories->click();
    } else {
        m_ui.radioShowMenuIconOnly->click();
    }
}

void LancelotAppletConfig::setClickActivation(bool value)
{
    if (value) {
        m_ui.radioActivationClick->click();
    } else {
        m_ui.radioActivationHover->click();
    }
}

void LancelotAppletConfig::setIcon(QString icon)
{
    if (icon == "lancelot") {
        m_ui.radioIconLancelot->click();
    } else if (icon == "kde") {
        m_ui.radioIconKDE->click();
    } else if (icon == "start-here") {
        m_ui.radioIconStartHere->click();
    } else {
        m_ui.radioIconCustom->click();
        m_ui.iconCustom->setIcon(icon);
    }
}
