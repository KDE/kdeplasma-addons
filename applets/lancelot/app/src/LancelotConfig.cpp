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
#include "models/SystemActions.h"

LancelotConfig::LancelotConfig()
    : systemButtonActionsMenu(NULL), clickedSystemButton(NULL),
        m_config("lancelotrc"), m_mainConfig(&m_config, "Main")
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

    connect(buttonSystem1, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem2, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem3, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));

    loadConfig();
}

void LancelotConfig::systemButtonClicked()
{
    clickedSystemButton = static_cast < QPushButton * > (sender());
    if (!systemButtonActionsMenu) {
        systemButtonActionsMenu = new QMenu();
        Models::SystemActions * model = Models::SystemActions::instance();
        foreach (QString id, model->actions()) {
            systemButtonActionsMenu->addAction(
                    model->actionIcon(id),
                    model->actionTitle(id),
                    this, SLOT(systemButtonActionsMenuClicked()))
                ->setData(id);
        }
    }

    systemButtonActionsMenu->exec(QCursor::pos());
}

void LancelotConfig::systemButtonActionsMenuClicked()
{
    QAction * action = static_cast < QAction * > (sender());

    systemButtonActions[clickedSystemButton] =
        action->data().toString();
    setButtonData(clickedSystemButton);
}

void LancelotConfig::setButtonData(QPushButton * button)
{
    button->setText(Models::SystemActions::instance()->actionTitle(
                systemButtonActions[button]));
    button->setIcon(Models::SystemActions::instance()->actionIcon(
                systemButtonActions[button]));
}

void LancelotConfig::loadConfig()
{
    m_config.reparseConfiguration();
    setActivationMethod(
            (LancelotConfig::ActivationMethod)
            m_mainConfig.readEntry("activationMethod", (int)NoClick));
    setAppbrowserColumnLimitted(m_mainConfig.readEntry("appbrowserColumnLimitted", false));
    setAppbrowserReset(m_mainConfig.readEntry("appbrowserReset", true));

    // SystemButtons loading
    systemButtonActions[buttonSystem1] = m_mainConfig.readEntry("systemButton1Action", "lock-screen");
    systemButtonActions[buttonSystem2] = m_mainConfig.readEntry("systemButton2Action", "menu-leave");
    systemButtonActions[buttonSystem3] = m_mainConfig.readEntry("systemButton3Action", "menu-switch-user");

    setButtonData(buttonSystem1);
    setButtonData(buttonSystem2);
    setButtonData(buttonSystem3);
}

void LancelotConfig::saveConfig()
{
    m_mainConfig.writeEntry("activationMethod", (int)activationMethod());
    m_mainConfig.writeEntry("appbrowserColumnLimitted", appbrowserColumnLimitted());
    m_mainConfig.writeEntry("appbrowserReset", appbrowserReset());

    m_mainConfig.writeEntry("systemButton1Action", systemButtonActions[buttonSystem1]);
    m_mainConfig.writeEntry("systemButton2Action", systemButtonActions[buttonSystem2]);
    m_mainConfig.writeEntry("systemButton3Action", systemButtonActions[buttonSystem3]);

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

