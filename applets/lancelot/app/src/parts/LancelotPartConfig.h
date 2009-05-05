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

#ifndef LANCELOT_PART_CONFIG_H
#define LANCELOT_PART_CONFIG_H

#include <ui_LancelotPartConfigBase.h>
#include <lancelot/lancelot.h>

class LancelotPartConfig: public Ui::LancelotPartConfigBase {
public:
    void setupUi(QWidget * widget);

    bool iconClickActivation() const;
    QString icon() const;

    bool contentsClickActivation() const;
    Lancelot::ExtenderPosition contentsExtenderPosition() const;

    void setIconClickActivation(bool value);
    void setIcon(const QString & icon);
    void setIcon(const QIcon & icon);

    void setContentsClickActivation(bool value);
    void setContentsExtenderPosition(Lancelot::ExtenderPosition position);

    QButtonGroup * qbgIcon;
    QButtonGroup * qbgContents;
    QButtonGroup * qbgContentsExtenderPosition;
};

#endif /* LANCELOT_LAUNCHER_APPLET_CONFIG_H_ */
