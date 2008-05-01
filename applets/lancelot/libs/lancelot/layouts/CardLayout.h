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

#ifndef LANCELOT_CARD_LAYOUT_H_
#define LANCELOT_CARD_LAYOUT_H_

#include <lancelot/lancelot_export.h>

#include <plasma/layouts/layout.h>
#include <plasma/widgets/widget.h>
#include <cmath>
#include <QMap>

namespace Lancelot
{

class LANCELOT_EXPORT CardLayout : public Plasma::Layout
{
public:
    virtual Qt::Orientations expandingDirections() const;

    explicit CardLayout(LayoutItem * parent = 0);
    virtual ~CardLayout();

    QSizeF sizeHint() const;

    void addItem (Plasma::LayoutItem * item);
    void addItem (Plasma::Widget * widget,const QString & id);

    void removeItem (Plasma::LayoutItem * item);
    void removeItem (const QString & id);

    virtual int count() const;
    virtual int indexOf(Plasma::LayoutItem * item) const;
    virtual Plasma::LayoutItem * itemAt(int i) const;
    virtual Plasma::LayoutItem * takeAt(int i);

    void show(const QString & id);
    void hideAll();

protected:
    void relayout();
    void releaseManagedItems();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_CARD_LAYOUT_H_ */

