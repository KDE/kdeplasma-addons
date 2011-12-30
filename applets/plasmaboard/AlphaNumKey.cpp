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


#include "AlphaNumKey.h"
#include "Helpers.h"
#include <QPainter>
#include <plasma/theme.h>

AlphaNumKey::AlphaNumKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode):
    BoardKey(relativePosition, relativeSize, keycode)
{
    setLabel(0);
}

AlphaNumKey::~AlphaNumKey()
{

}

QString AlphaNumKey::label() const
{
    return m_label;
}

void AlphaNumKey::paint(QPainter *painter)
{
    BoardKey::paint(painter);
    if (!m_label.isEmpty()) {
        paintLabel(painter);
    }
}

void AlphaNumKey::paintLabel(QPainter *painter)
{
    painter->save();
    int fontSize = qMin(size().width(), size().height()) / 2;
    painter->setFont(QFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), fontSize));
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    painter->drawText(rect(), Qt::AlignCenter, m_label);
    painter->restore();
}

void AlphaNumKey::setLabel(int level)
{
    setLabel(Helpers::mapToUnicode(keysymbol(level)));
}

void AlphaNumKey::setLabel(const QString &label)
{
    m_label = label;
}

void AlphaNumKey::switchKey(bool isLevel2, bool isAlternative, bool isLocked)
{
    if (isLocked) {
        isLevel2 = !isLevel2;
    }
    if (isLevel2) {
        isAlternative ? setLabel(4) : setLabel(1);
    } else {
        setLabel(0);
    }
}


