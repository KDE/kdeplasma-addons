/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <QToolButton>

#include <Lancelot/Lancelot>
#include <Lancelot/PopupList>

class LancelotPartConfig: public QObject, public Ui::LancelotPartConfigBase {
    Q_OBJECT

public:
    void setupUi(QWidget * widget);

    bool iconClickActivation() const;
    bool showSearchBox() const;
    QString icon() const;

    bool contentsClickActivation() const;
    Lancelot::ExtenderPosition contentsExtenderPosition() const;

    void setIconClickActivation(bool value);
    void setIcon(const QString & icon);
    void setIcon(const QIcon & icon);

    void setContentsClickActivation(bool value);
    void setContentsExtenderPosition(Lancelot::ExtenderPosition position);
    void setShowSearchBox(bool value);

    void setPartData(const QString & data);
    QString partData() const;

    void addItem(const QString & itemData);
    void setItemData(
        QListWidgetItem * item, const QString & itemData);

    QButtonGroup * qbgIcon;
    QButtonGroup * qbgContents;
    QButtonGroup * qbgContentsExtenderPosition;

    QToolButton  * buttonContentsRemove;

signals:
    void contentsChanged();

public Q_SLOTS:
    void buttonContentsAddClicked();
    void buttonContentsRemoveClicked();
    void buttonContentsAddItemSelected(int index);

    void listModelsItemClicked(QListWidgetItem * item);
    void listModelsItemSelectionChanged();

private:
    Lancelot::PopupList * popup;

};

#endif /* LANCELOT_LAUNCHER_APPLET_CONFIG_H_ */
