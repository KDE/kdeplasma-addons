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

AlphaNumKey::AlphaNumKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode):
        BoardKey(relativePosition, relativeSize, keycode){

    setLabel(0);
}

AlphaNumKey::~AlphaNumKey() {

}

const QString AlphaNumKey::label() const
{
    return m_label;
}

void AlphaNumKey::paint(QPainter *painter)
{
    BoardKey::paint(painter);
    painter->save();
    setUpPainter(painter);
    painter->scale(1.0/(relativeSize().width()/size().width()), 1.0/(relativeSize().width()/size().width()));
    painter->drawText(QPoint(-80 * m_label.size(),100), m_label);
    painter->restore();
}

void AlphaNumKey::setLabel(int level)
{
    m_label = Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),level));
}

void AlphaNumKey::setLabel(QString &label)
{
    m_label = label;
}

void AlphaNumKey::switchKey(bool isLevel2, bool isAlternative, bool isLocked)
{
	if(isLocked){
		isLevel2 = !isLevel2;
	}
	if(isLevel2){
		isAlternative ? setLabel(4) : setLabel(1);
	}
	else{
		setLabel(0);
		/*isAlternative ?
		setText(QChar(Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),0)))) :
		setText(QChar(Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),0))));*/
    }
}


