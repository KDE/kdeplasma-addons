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

class BoardKey
{

public:
    BoardKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode);
    virtual ~BoardKey();

    bool contains(const QPoint &point) const;
    bool intersects(const QRectF &rect) const;
    unsigned int keycode() const;
    unsigned int keysymbol(int level) const;
    virtual QString label() const;
    virtual void paint(QPainter *painter);
    QPoint position() const;

    /**
      * called when button is pressed
      */
    virtual void pressed();

    /**
     * @return true if this key can repeat
     */
    virtual bool repeats() const;

    /**
      * This should be called when the key is pressed for a longer time with out releasing or moving the pointer
      */
    virtual void pressRepeated();
    QRectF rect() const;
    QSize relativeSize() const;

    /**
      * called when button is released after being pressed
      * This will usually create an X-Event. It may be a key release for functional keys
      * or a press and release for alphanumeric keys
      */
    virtual void released();

    /**
      * Called to make sticky (switch) buttons accepting unpressable again.
      */
    virtual void reset();
    void setKeycode(unsigned int keycode);
    virtual bool setPixmap(QPixmap *pixmap);
    QSize size() const;
    void updateDimensions(double factor_x, double factor_y);

protected:
    void sendKey();
    virtual void sendKeyPress();
    virtual void sendKeyRelease();
    void setUpPainter(QPainter *painter) const;

private:
    unsigned int m_keycode;
    QPixmap* m_pixmap;
    QPoint m_relativePosition;
    QRectF m_rect;
    QSize m_relativeSize;

};

#endif /* BOARDKEY_H */
