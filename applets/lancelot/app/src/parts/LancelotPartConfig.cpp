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

#include "LancelotPartConfig.h"

void LancelotPartConfig::setupUi(QWidget * widget)
{
    Ui::LancelotPartConfigBase::setupUi(widget);

    qbgIcon = new QButtonGroup(widget);
    qbgIcon->addButton(radioIconActivationClick);
    qbgIcon->addButton(radioIconActivationHover);

    qbgContents = new QButtonGroup(widget);
    qbgContents->addButton(radioContentsActivationClick);
    qbgContents->addButton(radioContentsActivationExtender);

    qbgContentsExtenderPosition = new QButtonGroup(widget);
    qbgContentsExtenderPosition->addButton(radioContentsExtenderPositionLeft);
    qbgContentsExtenderPosition->addButton(radioContentsExtenderPositionRight);
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

void LancelotPartConfig::setIcon(const QString & icon)
{
    iconChooser->setIcon(icon);
}

void LancelotPartConfig::setIcon(const QIcon & icon)
{
    iconChooser->setIcon(icon);
}
