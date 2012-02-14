/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOTAPP_MODELS_APPLICATIONS_P_H
#define LANCELOTAPP_MODELS_APPLICATIONS_P_H

#include "BaseModel.h"
#include "Applications.h"

namespace Lancelot {
namespace Models {

class Applications::Private: public QObject {
    Q_OBJECT
public:
    Private(Applications * parent);
    ~Private();

    void load();
    void clear();

    class ApplicationData {
    public:
        QString name, description;
        QIcon icon;
        QString desktopFile;
    };

    QList < Applications * > submodels;
    QList < ApplicationData > items;

    enum Data {
        Title, Description
    };

    QString data(int index, Data what);

    QString root;
    QString title;
    QIcon icon;
    bool flat;

    QString lastTitle, lastDescription;
    int lastIndex;

public Q_SLOTS:
    void sycocaUpdated(const QStringList & resources);

private:
    Applications * const q;

};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_APPLICATIONS_P_H */
