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


#ifndef ALPHANUMKEY_H
#define ALPHANUMKEY_H

#include "BoardKey.h"

class AlphaNumKey: public BoardKey
{


public:
    AlphaNumKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode);
    virtual ~AlphaNumKey();

    virtual QString label() const;
    virtual void paint(QPainter *painter);

    /**
      * Sets button text regarding the key level
      */
    void setLabel(int level);
    void setLabel(const QString &label);
    void switchKey(bool isLevel2, bool isAlternative, bool isLocked);

protected:
    virtual void paintLabel(QPainter *painter);

private:
    QString m_label;

};

#endif /* ALPHANUMKEY_H */
