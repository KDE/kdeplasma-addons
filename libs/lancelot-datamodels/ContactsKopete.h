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

#ifndef LANCELOTAPP_MODELS_CONTACTSKOPETE_H
#define LANCELOTAPP_MODELS_CONTACTSKOPETE_H

#include <lancelot/lancelot_export.h>

#include "BaseModel.h"

namespace Lancelot {
namespace Models {

/**
 * Model containing online contacts from Kopete
 */
class LANCELOT_EXPORT ContactsKopete : public BaseModel {
    Q_OBJECT
public:
    /**
     * Creates a new instance of ContactsKopete
     */
    ContactsKopete();

    /**
     * Destroys this ContactsKopete
     */
    ~ContactsKopete();

    L_Override QString selfShortTitle() const;

protected:
    /**
     * Opens a chat window for the specified contact
     * @param index index of the ocntact in model
     */
    void activate(int index);

    /**
     * Loads the model data
     */
    void load();

    /**
     * Loads the model data
     * @param forceReload if true, all contacts will be reloaded
     */
    void load(bool forceReload);

    /**
     * Updated contact data
     * @param contactId id of the contact to update
     */
    void updateContactData(const QString & contactId);

    void timerEvent(QTimerEvent * event);

protected Q_SLOTS:
    void contactChanged(const QString & contactId);

    void kopeteServiceOwnerChanged(const QString & serviceName, const QString & oldOwner, const QString & newOwner);

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif // LANCELOTAPP_MODELS_CONTACTSKOPETE_H


