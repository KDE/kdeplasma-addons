/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOT_CUSTOM_LIST_VIEW_MODELS_H_
#define LANCELOT_CUSTOM_LIST_VIEW_MODELS_H_

#include <lancelot/lancelot_export.h>

#include <QObject>

namespace Lancelot {

/**
 * The most abstract list interface
 */
class LANCELOT_EXPORT AbstractListModel: public QObject {
    Q_OBJECT
public:
    AbstractListModel();
    virtual ~AbstractListModel();

    /**
     * @returns the number of items in model
     */
    virtual int size() const = 0;

Q_SIGNALS:
    /**
     * This signal is emitted when the model is updated and the update
     * is too complex to explain using itemInserted, itemDeleted and
     * itemAltered methods
     */
    void updated();

    /**
     * This signal is emitted when an item is inserted into the model
     * @param index place where the new item is inserted
     */
    void itemInserted(int index);

    /**
     * This signal is emitted when an item is deleted from the model
     * @param index index of the deleted item
     */
    void itemDeleted(int index);

    /**
     * This signal is emitted when an item is altered
     * @param index index of the altered item
     */
    void itemAltered(int index);
};

} // namespace Lancelot

#endif // LANCELOT_CUSTOM_LIST_VIEW_MODELS_H_


