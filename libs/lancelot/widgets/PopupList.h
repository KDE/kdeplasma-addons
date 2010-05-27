/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <Plasma/Dialog>

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>
#include <lancelot/Global.h>

namespace Lancelot {

class ActionListView;
class ActionListModel;

/**
 * The list that pops up in its own window.
 * Note: Don't save pointers to this object, it can destroy
 * itself. Use QPointer.
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT PopupList: public Plasma::Dialog {
    Q_OBJECT

    Q_PROPERTY ( int closeTimeout READ closeTimeout WRITE setCloseTimeout )
    Q_PROPERTY ( SublevelOpenAction sublevelOpenAction READ sublevelOpenAction WRITE setSublevelOpenAction )

public:
    /**
     * This enum represents behaviour patterns when an
     * item with a submenu is activated
     */
    enum SublevelOpenAction {
        NoAction = 0, ///< Do nothing
        PopupNew,     ///< Pops a new list
        OpenInside    ///< Opens the sublist inside the current one
    };

    /**
     * Creates a new Lancelot::PopupList
     * @param parent parent item
     */
    explicit PopupList(QWidget * parent = 0, Qt::WindowFlags f =  Qt::Window);

    /**
     * Destroys Lancelot::PopupList
     */
    virtual ~PopupList();

    /**
     * Sets the timer for auto-closing when the popup
     * is not hovered.
     * @param timeout in milliseconds
     */
    void setCloseTimeout(int timeout);

    /**
     * @returns the timeout
     */
    int closeTimeout() const;

    /**
     * Sets the action for opening a sublevel
     * @param action new action
     */
    void setSublevelOpenAction(SublevelOpenAction action);

    /**
     * @returns action for opening a sublevel
     */
    SublevelOpenAction sublevelOpenAction() const;

    /**
     * Sets the model for the popup list. You can use
     * ActionTreeModel as well as ActionListModel
     */
    void setModel(ActionListModel * model);

    /**
     * Pops out the list
     * @param p pop-up location
     */
    void exec(const QPoint & p);

//    /**
//     * Shows the widget
//     */
//    void show();

protected:
    L_Override void showEvent(QShowEvent * event);
    L_Override void hideEvent(QHideEvent * event);

    L_Override void enterEvent(QEvent * event);
    L_Override void leaveEvent(QEvent * event);
    L_Override void timerEvent(QTimerEvent * event);
    L_Override bool eventFilter(QObject * object, QEvent * event);

    /**
     * Pops out the list
     * @param p pop-up location
     * @param parent parent list
     */
    void exec(const QPoint & p, PopupList * parent);

    /**
     * Moves the list to the specified point
     */
    void moveTo(const QPoint & p);

    /**
     * @returns the parent PopupList
     */
    PopupList * parentList() const;

Q_SIGNALS:
    void activated(int index);

public Q_SLOTS:
    /**
     * Requests the PopupList to be resized depending on the number of items
     */
    void updateSize();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_POPUPLIST_H */

