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

#ifndef LANCELOTAPP_MODELS_CONTACTSKOPETE_H
#define LANCELOTAPP_MODELS_CONTACTSKOPETE_H

#include "kopete_interface.h"
#include "BaseModel.h"

namespace Models {

class ContactsKopete : public BaseModel {
    Q_OBJECT
public:
    ContactsKopete();
    ~ContactsKopete();

protected:
    void activate(int index);
    void load();
    void load(bool forceReload);
    void updateContactData(const QString & contactId);
    void timerEvent(QTimerEvent * event);

protected Q_SLOTS:
    void contactChanged(const QString & contactId);

private:
    org::kde::Kopete * m_interface;
    QBasicTimer m_delayTimer;
    QBasicTimer m_checkRunningTimer;
    QStringList m_contactsToUpdate;
    QString m_kopeteAvatarsDir;
    bool m_kopeteRunning : 1;
    bool m_noOnlineContacts : 1;
};

} // namespace Models

#endif // LANCELOTAPP_MODELS_CONTACTSKOPETE_H


