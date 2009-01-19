/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOT_POPUPLIST_H
#define LANCELOT_POPUPLIST_H

#include <KDE/KDebug>

#include <plasma/dialog.h>

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>

#include <lancelot/Global.h>

namespace Lancelot {

/**
 * The widget that pops up in its own window
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT PopupList: public Plasma::Dialog {
    Q_OBJECT

public:
    /**
     * Creates a new Lancelot::PopupList
     * @param parent parent item
     */
    PopupList(QWidget * parent = 0, Qt::WindowFlags f =  Qt::Window);

    /**
     * Destroys Lancelot::PopupList
     */
    virtual ~PopupList();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_POPUPLIST_H */

