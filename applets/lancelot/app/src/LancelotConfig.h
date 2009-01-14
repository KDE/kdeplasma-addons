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

#ifndef LANCELOTCONFIG_H
#define LANCELOTCONFIG_H

#include "ui_LancelotConfigBase.h"
#include <KConfig>
#include <KConfigGroup>
#include <QButtonGroup>

class LancelotConfig: public Ui::LancelotConfigBase {
public:
    LancelotConfig();

    void setupUi(QWidget * widget);

    void loadConfig();
    void saveConfig();

    enum ActivationMethod {
        Click = 0,
        Classic = 1,
        NoClick = 2
    };

    ActivationMethod activationMethod();
    void setActivationMethod(ActivationMethod method);

    bool appbrowserColumnLimitted();
    void setAppbrowserColumnLimitted(bool value);

    bool appbrowserReset();
    void setAppbrowserReset(bool value);

    QButtonGroup * qbgActivationMethod;
    QButtonGroup * qbgAppbrowserColumnLimit;

    KConfig      m_config;
    KConfigGroup m_mainConfig;
};

#endif // LANCELOTCONFIG_H


