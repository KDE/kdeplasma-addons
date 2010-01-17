/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <lancelot/lancelot_export.h>

#include <taskmanager/taskmanager.h>
#include "BaseModel.h"

using TaskManager::TaskPtr;

namespace Lancelot {
namespace Models {

/**
 * Lists folders in KMail that have unread mail in them
 */
class LANCELOT_EXPORT MessagesKmail : public BaseModel {
    Q_OBJECT
public:
    /**
     * Creates a new MessagesKmail instance
     */
    MessagesKmail();

    /**
     * Destroys this MessagesKmail
     */
    ~MessagesKmail();

    void timerEvent(QTimerEvent * event);

protected:
    /**
     * Opens the message folder
     * @param index index of the folder to open
     */
    void activate(int index);

    void load();

private Q_SLOTS:
    void unreadCountChanged();

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif // LANCELOTAPP_MODELS_MESSAGESKMAIL_H


