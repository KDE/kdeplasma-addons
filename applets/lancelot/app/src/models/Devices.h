/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
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

#ifndef LANCELOT_MODELS_DEVICES_H_
#define LANCELOT_MODELS_DEVICES_H_

#include "../ActionListViewModels.h"
#include <solid/device.h>
#include <solid/storageaccess.h>

namespace Lancelot {
namespace Models {

class Devices : public StandardActionListViewModel {
    Q_OBJECT
public:
    Devices();
    virtual ~Devices();

private slots:
    void deviceRemoved(const QString & udi);
    void deviceAdded(const QString & udi);
    void freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable);
    void udiAccessibilityChanged(bool accessible);
    
protected:
    void activate(int index);
    
private:
    void loadDevices();
    void addDevice(const Solid::Device & device);
    
    QMap < const Solid::StorageAccess *, QString> m_udis;
};

}
}

#endif /* LANCELOT_MODELS_DEVICES_H_ */
