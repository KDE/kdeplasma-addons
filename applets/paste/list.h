/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIST_HEADER
#define LIST_HEADER

#include <Plasma/Dialog>
#include "ui_list.h"

class QStandardItemModel;
class ConfigData;
class QWidget;

class ListForm : public QWidget, public Ui::ListForm
{
    Q_OBJECT
public:
    enum SpecificRoles {
        DataRole = Qt::UserRole + 1,
        SubTitleRole,
        SubTitleMandatoryRole
    };

    ListForm(QWidget *parent = 0);
    virtual ~ListForm();

    void setHideAfterClick(bool hide);

public slots:
    void setData(const ConfigData &data);

protected slots:
    void clicked(const QModelIndex &index);
    void paste();
    void themeChanged();

signals:
    void textCopied();

private:
    QStandardItemModel *m_listModel;
    bool m_hide;
    QKeySequence m_pasteKey;
    const ConfigData *cfg;
};

#endif
