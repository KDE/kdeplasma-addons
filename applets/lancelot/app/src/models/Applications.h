/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLICATIONS_H_
#define APPLICATIONS_H_

#include "BaseModel.h"
#include "../PassagewayViewModels.h"

namespace Lancelot {
namespace Models {

class Applications : public PassagewayViewModel {
    Q_OBJECT
public:
    Applications(QString root = "", QString title = "", KIcon * icon = NULL);
    virtual ~Applications();

    QString title(int index) const;
    QString description(int index) const;
    KIcon * icon(int index) const;
    bool isCategory(int index) const;

    int size() const;

    PassagewayViewModel * child(int index);
    QString modelTitle() const;
    KIcon * modelIcon()  const;

public slots:
    void activated(int index);

private:
    void load();
    void clear();
    
    class ApplicationData {
    public:
        QString name, description;
        KIcon * icon;
        QString desktopFile;
    };
    
    QList < Applications * > m_submodels;
    QList < ApplicationData > m_items;
    
    QString m_root;
    QString m_title;
    KIcon * m_icon;
    bool m_loaded;
};

}
}

#endif /* APPLICATIONS_H_*/
