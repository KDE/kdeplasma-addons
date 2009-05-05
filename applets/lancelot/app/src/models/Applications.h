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

#ifndef LANCELOTAPP_MODELS_APPLICATIONS_H
#define LANCELOTAPP_MODELS_APPLICATIONS_H

#include "BaseModel.h"
#include <lancelot/models/ActionTreeModel.h>

namespace Models {

class Applications : public Lancelot::ActionTreeModel {
    Q_OBJECT
public:
    explicit Applications(QString root = "", QString title = "", QIcon icon = QIcon(), bool flat = false);
    virtual ~Applications();

    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override bool isCategory(int index) const;

    L_Override int size() const;

    L_Override ActionTreeModel * child(int index);
    L_Override QString selfTitle() const;
    L_Override QIcon selfIcon()  const;

    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);

    L_Override QMimeData * mimeData(int index) const;
    L_Override void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override QMimeData * selfMimeData() const;

public Q_SLOTS:
    virtual void activate(int index);

private Q_SLOTS:
    void sycocaUpdated(const QStringList & resources);

private:
    void load();
    void clear();

    class ApplicationData {
    public:
        QString name, description;
        QIcon icon;
        QString desktopFile;
    };

    QList < Applications * > m_submodels;
    QList < ApplicationData > m_items;

    QString m_root;
    QString m_title;
    QIcon m_icon;
    bool m_flat;
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_APPLICATIONS_H */
