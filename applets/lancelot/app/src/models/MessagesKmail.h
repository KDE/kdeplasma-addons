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

#ifndef LANCELOTAPP_MODELS_MESSAGESKMAIL_H
#define LANCELOTAPP_MODELS_MESSAGESKMAIL_H

#include "kmail_interface.h"
#include "kmailfolder_interface.h"
#include <taskmanager/taskmanager.h>
#include "BaseModel.h"

using TaskManager::TaskPtr;

namespace Models {

class MessagesKmail : public BaseModel {
    Q_OBJECT
public:
    MessagesKmail();
    ~MessagesKmail();

    void timerEvent(QTimerEvent * event);

protected:
    void activate(int index);
    void load();

private Q_SLOTS:
    void unreadCountChanged();

private:
    org::kde::kmail::kmail * m_interface;
    org::kde::kmail::folder * m_folderinterface;
    QBasicTimer m_timer;
    QString m_kopeteAvatarsDir;
    bool m_kmailRunning : 1;
    bool m_dataValid : 1;
};

} // namespace Models

#endif // LANCELOTAPP_MODELS_MESSAGESKMAIL_H


