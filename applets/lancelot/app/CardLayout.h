/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CARDLAYOUT_H_
#define CARDLAYOUT_H_

#include <plasma/widgets/layout.h>
#include <plasma/widgets/widget.h>
#include <cmath>
#include <QMap>

namespace Lancelot
{

class CardLayout : public Plasma::Layout
{
    
public:
    // reimplemented
    virtual Qt::Orientations expandingDirections() const;
    
    explicit CardLayout(LayoutItem * parent = 0);
    virtual ~CardLayout();
    
    virtual QRectF geometry() const;
    void setGeometry(const QRectF& geometry);

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

private:
    QList < Plasma::LayoutItem * > m_items;
    QMap < QString, Plasma::Widget * > m_widgets;
    Plasma::Widget * m_shown;
    QRectF m_geometry;
};

}

#endif /*CARDLAYOUT_H_*/
