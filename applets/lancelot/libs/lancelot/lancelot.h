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

#ifndef LIBLANCELOT_H_
#define LIBLANCELOT_H_

#include <QWidget>
#include <QMetaObject>

namespace Lancelot
{

enum ExtenderPosition {
    NoExtender = 0,
    RightExtender = 1,
    LeftExtender = 2,
    TopExtender = 3,
    BottomExtender = 4
};

enum ActivationMethod {
    HoverActivate = 0,
    ClickActivate = 1,
    ExtenderActivate = 2
};

#define MAX_WIDGET_SIZE QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)

#define L_WIDGET                                           \
    private:                                               \
        const QMetaObject * L_INITD_META_OBJECT;           \
    protected:                                             \
        virtual bool L_isInitialized() const {             \
            return L_metaObject() == metaObject();         \
        }                                                  \
        virtual void L_setInitialized() {                  \
            L_INITD_META_OBJECT = metaObject();            \
        }                                                  \
        virtual const QMetaObject * L_metaObject() const { \
            return L_INITD_META_OBJECT;                    \
        }
//    public:
//        virtual void debug() const {
//            kDebug() << " 1 " << L_metaObject()->className();
//            kDebug() << " 2 " << metaObject()->className();
//            kDebug() << " 3 " << (long int)(L_INITD_META_OBJECT->className());
//            kDebug() << " 4 " << (long int)(metaObject()->className());
//        }

#define L_WIDGET_IS_INITIALIZED  L_isInitialized()
#define L_WIDGET_SET_INITIALIZED L_setInitialized(); updateGeometry();

} // namespace Lancelot

#endif // LIBLANCELOT_H_

