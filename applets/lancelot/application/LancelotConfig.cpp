/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <KRun>
#include <KUrl>
#include <KServiceTypeTrader>
#include <KService>
#include <KPluginInfo>

#include <Lancelot/Models/NewDocuments>
#include <Lancelot/Models/SystemActions>
#include <Lancelot/Models/SystemServices>

LancelotConfig::LancelotConfig()
    : systemButtonActionsMenu(NULL), clickedSystemButton(NULL),
        m_config("lancelotrc"), m_mainConfig(&m_config, "Main")
{
}

void LancelotConfig::setupUi(QWidget * widget)
{
    Ui::LancelotConfigBase::setupUi(widget);

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QList < KPluginInfo > runnerInfo = KPluginInfo::fromServices(offers);

    m_searchPlugins = new KPluginSelector(widget);
    m_searchPlugins->addPlugins(
            runnerInfo,
            KPluginSelector::ReadConfigFile,
            i18n("Available Features"),
            QString(),
            KSharedConfig::openConfig("lancelotrc")
        );
    tabWidget->addTab(m_searchPlugins, i18n("Search"));

    connect(m_searchPlugins, SIGNAL(changed(bool)), this, SIGNAL(searchPluginChanged()));

    tabWidget->setCurrentIndex(0);

    qbgActivationMethod = new QButtonGroup(widget);
    qbgActivationMethod->addButton(radioActivationClick);
    qbgActivationMethod->addButton(radioActivationClassic);
    qbgActivationMethod->addButton(radioActivationNoClick);

    qbgAppbrowserColumnLimit = new QButtonGroup(widget);
    qbgAppbrowserColumnLimit->addButton(radioAppBrowserNoColumnLimit);
    qbgAppbrowserColumnLimit->addButton(radioAppBrowserTwoColumnLimit);
    qbgAppbrowserColumnLimit->addButton(radioAppBrowserCascade);

    connect(buttonSystem1, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem2, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem3, SIGNAL(clicked()), this, SLOT(systemButtonClicked()));

    connect(buttonNewDocumentsEdit, SIGNAL(clicked()),
            this, SLOT(buttonNewDocumentsEditClicked()));
    connect(buttonSystemApplicationsEdit, SIGNAL(clicked()),
            this, SLOT(buttonSystemApplicationsEditClicked()));

    loadConfig();
}

void LancelotConfig::systemButtonClicked()
{
    clickedSystemButton = static_cast < QPushButton * > (sender());
    if (!systemButtonActionsMenu) {
        systemButtonActionsMenu = new QMenu();
        Lancelot::Models::SystemActions * model = Lancelot::Models::SystemActions::self();
        foreach (const QString &id, model->actions()) {
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
    emit(systemButtonChanged());
}

void LancelotConfig::setButtonData(QPushButton * button)
{
    button->setText(Lancelot::Models::SystemActions::self()->actionTitle(
                systemButtonActions[button]));
    button->setIcon(Lancelot::Models::SystemActions::self()->actionIcon(
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
    setAppbrowserPopupSubmenus(m_mainConfig.readEntry("appbrowserPopupSubmenus", false));

    // SystemButtons loading
    systemButtonActions[buttonSystem1] = m_mainConfig.readEntry("systemButton1Action", "lock-screen");
    systemButtonActions[buttonSystem2] = m_mainConfig.readEntry("systemButton2Action", "menu-leave");
    systemButtonActions[buttonSystem3] = m_mainConfig.readEntry("systemButton3Action", "menu-switch-user");

    setButtonData(buttonSystem1);
    setButtonData(buttonSystem2);
    setButtonData(buttonSystem3);

    checkKeepOpen->setChecked(m_mainConfig.readEntry("enableKeepOpen", false));
    setEnableUsageStatistics(m_mainConfig.readEntry("enableUsageStatistics", true));
    setAppNameFirst(m_mainConfig.readEntry("applicationNameFirst", true));

    m_searchPlugins->load();
}

void LancelotConfig::saveConfig()
{
    m_mainConfig.writeEntry("activationMethod", (int)activationMethod());
    m_mainConfig.writeEntry("appbrowserColumnLimitted", appbrowserColumnLimitted());
    m_mainConfig.writeEntry("appbrowserPopupSubmenus", appbrowserPopupSubmenus());
    m_mainConfig.writeEntry("appbrowserReset", appbrowserReset());

    m_mainConfig.writeEntry("systemButton1Action", systemButtonActions[buttonSystem1]);
    m_mainConfig.writeEntry("systemButton2Action", systemButtonActions[buttonSystem2]);
    m_mainConfig.writeEntry("systemButton3Action", systemButtonActions[buttonSystem3]);

    m_mainConfig.writeEntry("enableUsageStatistics", enableUsageStatistics());
    m_mainConfig.writeEntry("enableKeepOpen", checkKeepOpen->isChecked());
    m_mainConfig.writeEntry("applicationNameFirst", appNameFirst());

    m_searchPlugins->save();

    m_mainConfig.sync();
}

LancelotConfig::ActivationMethod LancelotConfig::activationMethod() const
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

bool LancelotConfig::appbrowserReset() const
{
    return (checkAppBrowserReset->isChecked());
}

void LancelotConfig::setAppbrowserReset(bool value)
{
    checkAppBrowserReset->setChecked(value);
}

bool LancelotConfig::appbrowserColumnLimitted() const
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

bool LancelotConfig::appbrowserPopupSubmenus() const
{
    return radioAppBrowserCascade->isChecked();
}

void LancelotConfig::setAppbrowserPopupSubmenus(bool value)
{
    if (value) {
        radioAppBrowserCascade->click();
    }
}

bool LancelotConfig::appNameFirst() const
{
    return checkAppNameFirst->isChecked();
}

void LancelotConfig::setAppNameFirst(bool value)
{
    checkAppNameFirst->setChecked(value);
}

bool LancelotConfig::enableUsageStatistics() const
{
    return checkUsageStatisticsEnable->isChecked();
}

void LancelotConfig::setEnableUsageStatistics(bool value)
{
    checkUsageStatisticsEnable->setChecked(value);
}

QStringList LancelotConfig::searchHistory() const
{
    return m_mainConfig.readEntry("searchHistory",
            QStringList());
}

void LancelotConfig::setSearchHistory(const QStringList & history)
{
    m_mainConfig.writeEntry("searchHistory", history);
    m_mainConfig.sync();
}

void LancelotConfig::buttonNewDocumentsEditClicked()
{
    new KRun(KUrl(
        Lancelot::Models::NewDocuments::path()
    ), 0);
}

void LancelotConfig::buttonSystemApplicationsEditClicked()
{
    new KRun(KUrl(
        Lancelot::Models::SystemServices::path()
    ), 0);
}

