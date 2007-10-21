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

#include "CardLayout.h"
#include <KDebug>

namespace Lancelot
{


CardLayout::CardLayout(LayoutItem * parent)
  : Plasma::Layout(parent), m_shown(NULL)
{
}

CardLayout::~CardLayout()
{}

Qt::Orientations CardLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

QRectF CardLayout::geometry() const {
    return m_geometry;
}

void CardLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry " << geometry;
        return;
    }



    QRectF g = geometry;
    g.setTopLeft(g.topLeft() + QPointF(margin(), margin()));
    g.setBottomRight(g.bottomRight() - QPointF(margin(), margin()));

    foreach (Plasma::LayoutItem * l, m_items) {
        l->setGeometry(g);
    }

    foreach (Plasma::Widget * l, m_widgets) {
        l->show();
        l->setGeometry(g);
        if (m_shown != l) l->hide();
    }

    m_geometry = geometry;
}

QSizeF CardLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    foreach (Plasma::LayoutItem * l, m_items) {
        hintHeight = qMax(l->sizeHint().height(), hintHeight);
        hintWidth  = qMax(l->sizeHint().width(), hintWidth);
    }

    foreach (Plasma::Widget * l, m_widgets) {
        hintHeight = qMax(l->sizeHint().height(), hintHeight);
        hintWidth  = qMax(l->sizeHint().width(), hintWidth);
    }

    return QSizeF(hintWidth, hintHeight);
}

void CardLayout::addItem (Plasma::LayoutItem * item) {
    if (!m_items.contains(item)) {
        m_items.append(item);
    }
}

void CardLayout::addItem (Plasma::Widget * widget, const QString & id) {
    if (widget) {
        m_widgets[id] = widget;
        // widget->hide(); // BUGS in QT
    } else {
        removeItem(id);
    }
}

void CardLayout::removeItem (Plasma::LayoutItem * item) {
    m_items.removeAll(item);
    QMutableMapIterator<QString, Plasma::Widget *> i(m_widgets);
    while (i.hasNext()) {
        i.next();
        if ((Plasma::LayoutItem *)i.value() == item) {
            i.remove();
        }
    }
}

void CardLayout::removeItem (const QString & id) {
    m_widgets.remove(id);
}


int CardLayout::count() const {
    return m_items.size() + m_widgets.size();
}

int CardLayout::indexOf(Plasma::LayoutItem * item) const {
    if (m_items.contains(item)) {
        return m_items.indexOf(item);
    } else {
        int i = m_items.size();
        foreach (Plasma::Widget * l, m_widgets) {
            if ((Plasma::LayoutItem *) l == item) {
                return i;
            }
            ++i;
        }
    }
    return -1;
}

Plasma::LayoutItem * CardLayout::itemAt(int i) const {
    if (i < m_items.size()) {
        return m_items[i];
    } else {
        i -= m_items.size();
        foreach (Plasma::Widget * l, m_widgets) {
            if (0 == i--) {
                return (Plasma::LayoutItem *)l;
            }
        }
    }
    return NULL;
}

Plasma::LayoutItem * CardLayout::takeAt(int i) {
    Plasma::LayoutItem * item = itemAt(i);
    removeItem(item);
    return item;
}

void CardLayout::show(const QString & id) {
    if (!m_widgets.contains(id)) return;
    if (m_shown == m_widgets[id]) return;
    if (m_shown) {
        m_shown->hide();
    }
    m_shown = m_widgets[id];
    m_shown->show();
}

void CardLayout::hideAll() {
    if (!m_shown) return;
    m_shown->hide();
    m_shown = NULL;
}

}
