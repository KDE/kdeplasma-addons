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

#ifndef LANCELOTAPP_MODELS_DEVICES_PH
#define LANCELOTAPP_MODELS_DEVICES_PH

#include "Devices.h"
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <QXmlStreamReader>

namespace Lancelot {
namespace Models {

class Devices::Private: public QObject {
    Q_OBJECT
public:
    Private(Devices * parent);

public Q_SLOTS:
    void deviceRemoved(const QString & udi);
    void deviceAdded(const QString & udi);
    void udiAccessibilityChanged(bool accessible, const QString & udi);

    void deviceSetupDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

    void tearDevice(const QString & udi);
    void setupDevice(const QString & udi, bool openAfterSetup);
    void showError();

public:
    void readXbel();
    void readItem();

    void addDevice(const Solid::Device & device);

    QColor combineColors(
        QColor c1, qreal f1,
        QColor c2, qreal f2
        );

    QColor colorForPercentage(qreal percentage);

    QString error;
    QStringList udis;
    Type filter;
    QXmlStreamReader xmlReader;

private:
    Devices * const q;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_DEVICES_H */
