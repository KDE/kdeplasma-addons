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

#ifndef LANCELOTAPP_MODELS_DEVICES_H
#define LANCELOTAPP_MODELS_DEVICES_H

#include <lancelot/lancelot_export.h>

#include "BaseModel.h"
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <QXmlStreamReader>

namespace Lancelot {
namespace Models {

class LANCELOT_EXPORT Devices : public BaseModel {
    Q_OBJECT
public:
    enum Type {
        Fixed = 1,
        Removable = 2,
        All = 0
    };

    Devices(Type filter = All);
    virtual ~Devices();

    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);
    L_Override QMimeData * mimeData(int index) const;

protected:
    void activate(int index);
    void load();

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_DEVICES_H */
