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

#ifndef LANCELOT_APPLET_CONFIG_H
#define LANCELOT_APPLET_CONFIG_H

#include <ui_LancelotAppletConfigBase.h>

class LancelotAppletConfig: public QObject, Ui::LancelotAppletConfigBase {
    Q_OBJECT
public:
    void setupUi(QWidget * widget);

    bool showCategories() const;
    bool showCategory(const QString & id) const;
    bool clickActivation() const;
    QStringList showingCategories(bool value) const;
    QString icon() const;

    void setShowCategories(bool value);
    void setShowingCategories(QStringList ids, bool value);

    void setShowCategory(const QString & id, bool value);
    void setShowAllCategories(bool value);

    void setClickActivation(bool value);
    void setIcon(const QString & icon);

    QButtonGroup * qbgIcons;
    QButtonGroup * qbgChooseIcon;
    QButtonGroup * qbgMenuActivation;

    QMap < QString, QListWidgetItem * > categories;
    QMap < QString, QListWidgetItem * > icons;
    QString customIcon;

signals:
    void settingChanged();

protected Q_SLOTS:
    void updateCard();
    void iconItemClicked();
};

#endif /* LANCELOT_LAUNCHER_APPLET_CONFIG_H_ */
