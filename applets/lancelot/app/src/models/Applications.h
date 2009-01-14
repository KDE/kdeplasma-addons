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
#include <lancelot/models/PassagewayViewModels.h>

namespace Models {

class Applications : public Lancelot::PassagewayViewModel {
    Q_OBJECT
public:
    Applications(QString root = "", QString title = "", QIcon icon = QIcon());
    virtual ~Applications();

    L_Override virtual QString title(int index) const;
    L_Override virtual QString description(int index) const;
    L_Override virtual QIcon icon(int index) const;
    L_Override virtual bool isCategory(int index) const;

    L_Override virtual int size() const;

    L_Override virtual PassagewayViewModel * child(int index);
    L_Override virtual QString modelTitle() const;
    L_Override virtual QIcon modelIcon()  const;

    L_Override virtual bool hasContextActions(int index) const;
    L_Override virtual void setContextActions(int index, QMenu * menu);
    L_Override virtual void contextActivate(int index, QAction * context);

    L_Override virtual QMimeData * mimeData(int index) const;
    L_Override virtual void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override virtual QMimeData * modelMimeData();

public Q_SLOTS:
    virtual void activate(int index);

private Q_SLOTS:
    void sycocaUpdated();

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
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_APPLICATIONS_H */
