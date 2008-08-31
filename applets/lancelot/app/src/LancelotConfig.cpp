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

#include "LancelotConfig.h"
#include <KConfigGroup>

LancelotConfig::LancelotConfig()
    : m_config("lancelotrc"), m_mainConfig(&m_config, "Main")
{
}

void LancelotConfig::setupUi(QWidget * widget)
{
    Ui::LancelotConfigBase::setupUi(widget);

    qbgActivationMethod = new QButtonGroup(widget);
    qbgActivationMethod->addButton(radioActivationClick);
    qbgActivationMethod->addButton(radioActivationClassic);
    qbgActivationMethod->addButton(radioActivationNoClick);

    qbgAppbrowserColumnLimit = new QButtonGroup(widget);
    qbgAppbrowserColumnLimit->addButton(radioAppBrowserNoColumnLimit);
    qbgAppbrowserColumnLimit->addButton(radioAppBrowserTwoColumnLimit);

    loadConfig();
}

void LancelotConfig::loadConfig()
{
    m_config.reparseConfiguration();
    setActivationMethod(
            (LancelotConfig::ActivationMethod)
            m_mainConfig.readEntry("activationMethod", (int)NoClick));
    setAppbrowserColumnLimitted(m_mainConfig.readEntry("appbrowserColumnLimitted", false));
    setAppbrowserReset(m_mainConfig.readEntry("appbrowserReset", true));
}

void LancelotConfig::saveConfig()
{
    m_mainConfig.writeEntry("activationMethod", (int)activationMethod());
    m_mainConfig.writeEntry("appbrowserColumnLimitted", appbrowserColumnLimitted());
    m_mainConfig.writeEntry("appbrowserReset", appbrowserReset());
    m_mainConfig.sync();
}

LancelotConfig::ActivationMethod LancelotConfig::activationMethod()
{
    if (radioActivationClick->isChecked()) {
        return LancelotConfig::Click;
    } else if (radioActivationClassic->isChecked()) {
        return LancelotConfig::Classic;
    } else {
        return LancelotConfig::NoClick;
    }
}

void LancelotConfig::setActivationMethod(LancelotConfig::ActivationMethod method)
{
    switch (method) {
    case Click:
        radioActivationClick->click();
        break;
    case Classic:
        radioActivationClassic->click();
        break;
    case NoClick:
        radioActivationNoClick->click();
        break;
    }
}

bool LancelotConfig::appbrowserReset()
{
    return (checkAppBrowserReset->isChecked());
}

void LancelotConfig::setAppbrowserReset(bool value)
{
    checkAppBrowserReset->setChecked(value);
}

bool LancelotConfig::appbrowserColumnLimitted()
{
    return (radioAppBrowserTwoColumnLimit->isChecked());
}

void LancelotConfig::setAppbrowserColumnLimitted(bool value)
{
    if (value) {
        radioAppBrowserTwoColumnLimit->click();
    } else {
        radioAppBrowserNoColumnLimit->click();
    }
}

