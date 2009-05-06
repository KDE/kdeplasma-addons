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

#ifndef LANCELOT_POPUPMENU_H
#define LANCELOT_POPUPMENU_H

#include <plasma/dialog.h>

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>

#include <lancelot/Global.h>
#include <lancelot/widgets/PopupList.h>

namespace Lancelot {

/**
 * The popup menu class
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT PopupMenu: public PopupList {
    Q_OBJECT

public:
    /**
     * Creates a new Lancelot::PopupMenu
     * @param parent parent item
     */
    explicit PopupMenu(QWidget * parent = 0, Qt::WindowFlags f =  Qt::Window);

    /**
     * Destroys Lancelot::PopupMenu
     */
    virtual ~PopupMenu();

    QAction * addAction(const QIcon & icon, const QString & title);
    void addAction(QAction * action);

    QAction * exec(const QPoint & p, QAction * action = 0);

protected Q_SLOTS:
    void actionChosen(int index);
    QAction * action(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_POPUPMENU_H */

