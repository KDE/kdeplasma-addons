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

#ifndef LANCELOT_MODELS_DEVICES_H_
#define LANCELOT_MODELS_DEVICES_H_

#include "BaseModel.h"
#include <solid/device.h>
#include <solid/storageaccess.h>

namespace Lancelot {
namespace Models {

class Devices : public BaseModel {
    Q_OBJECT
public:
    enum Type {
        Fixed = 1,
        Removable = 2,
        All = 0
    };

    Devices(Type filter = All);
    virtual ~Devices();

private slots:
    void deviceRemoved(const QString & udi);
    void deviceAdded(const QString & udi);
    void freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable);
    void udiAccessibilityChanged(bool accessible, const QString & udi);

    void deviceSetupDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

protected:
    void activate(int index);
    void load();

private:
    void addDevice(const Solid::Device & device);

    Type m_filter;
    //QMap < const Solid::StorageAccess *, QString> m_udis;
    //QStringList m_devicesMounting;
};

}
}

#endif /* LANCELOT_MODELS_DEVICES_H_ */
