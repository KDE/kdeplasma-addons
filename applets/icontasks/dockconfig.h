/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __DOCKCONFIG_H__
#define __DOCKCONFIG_H__

#include "ui_dockconfig.h"
#include <KDE/KWidgetItemDelegate>
#include <QtGui/QWidget>
#include <QtGui/QIcon>
#include <QtCore/QSet>

class KConfigDialog;
class KPushButton;
class QCheckBox;
class QListWidgetItem;

class DockConfigItemDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:

    DockConfigItemDelegate(QAbstractItemView *itemView, QObject *parent = 0);
    virtual ~DockConfigItemDelegate();

    int dependantLayoutValue(int value, int width, int totalWidth) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QList<QWidget*> createItemWidgets() const;
    void updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const;
    QFont titleFont(const QFont &baseFont) const;

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void itemToggled(bool e);
    void aboutClicked();

private:
    QCheckBox *checkBox;
    KPushButton *pushButton;
};

class DockConfig : public QWidget
{
    Q_OBJECT

    struct Entry {
        QString dir;
        QString script;
        QString name;
        QString description;
        QString icon;
        bool available;
        bool enabled;
        bool user;
        QString appName;
        QString dbusName;
    };

public:
    DockConfig(KConfigDialog *p);
    virtual ~DockConfig();

    bool isEnabled();
    QSet<QString> enabledHelpers();

Q_SIGNALS:
    void settingsModified();

public Q_SLOTS:
    void selectionChanged();
    void add();
    void fileSelected();
    void del();
    void enableWidgets(bool e);

private:
    QListWidgetItem * createItem(const Entry &e);

private:
    Ui::DockConfig ui;
};

#endif
