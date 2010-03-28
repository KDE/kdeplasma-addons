/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef BOARDKEY_H
#define BOARDKEY_H

#include <widget.h>

class BoardKey  {

public:
    BoardKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode);
    virtual ~BoardKey();

    const bool contains (const QPoint &point) const;
    const bool intersects (const QRectF &rect) const;
    const unsigned int getKeycode() const;
    virtual const QString label() const;
    virtual void paint(QPainter *painter);
    const QPoint position() const;
    /**
      * called when button is pressed
      */
    virtual void pressed();
    const QRectF rect() const;
    const QSize relativeSize() const;
    /**
      * called when button is pressed
      */
    virtual void released();
    /**
      * Called to "unpress" the button
      */
    void reset();
    void sendKeycode();
    void sendKeycodePress();
    void sendKeycodeRelease();
    void setPixmap(QPixmap *pixmap);
    const QSize size() const;
    void unpressed();
    void updateDimensions(double factor_x, double factor_y);

protected:
    void setUpPainter(QPainter *painter) const;

private:
    unsigned int m_keycode;
    QPixmap* m_pixmap;
    QPoint m_position;
    QPoint m_relativePosition;
    QRectF m_rect;
    QSize m_relativeSize;
    QSize m_size;

};

#endif /* BOARDKEY_H */
