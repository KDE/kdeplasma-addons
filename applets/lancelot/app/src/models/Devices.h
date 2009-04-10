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

#ifndef LANCELOTAPP_MODELS_DEVICES_H
#define LANCELOTAPP_MODELS_DEVICES_H

#include "BaseModel.h"
#include <solid/device.h>
#include <solid/storageaccess.h>

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

    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);

private slots:
    void deviceRemoved(const QString & udi);
    void deviceAdded(const QString & udi);
    void freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable);
    void udiAccessibilityChanged(bool accessible, const QString & udi);

    void deviceSetupDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

    void tearDevice(const QString & udi);
    void setupDevice(const QString & udi, bool openAfterSetup);

protected:
    void activate(int index);
    void load();

private:
    void addDevice(const Solid::Device & device);

    Type m_filter;
    //QMap < const Solid::StorageAccess *, QString> m_udis;
    //QStringList m_devicesMounting;
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_DEVICES_H */
