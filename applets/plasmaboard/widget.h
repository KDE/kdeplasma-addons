/****************************************************************************
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



#ifndef WIDGET_H_
#define WIDGET_H_

#include "BoardKey.h"
#include "tooltip.h"
#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/widgets/label.h>
#include <QTimer>
#include <QList>
#include "Helpers.h"

#define XK_TECHNICAL
#define XK_PUBLISHING
#define XK_LATIN1

#include <X11/keysym.h>

class AlphaNumKey;
class FuncKey;
class QGraphicsGridLayout;


class PlasmaboardWidget : public Plasma::Containment
{
    Q_OBJECT
    public:
        // Basic Create/Destroy
    	PlasmaboardWidget(QGraphicsWidget *parent);
        ~PlasmaboardWidget();

        // The paintInterface procedure paints the applet to screen
        void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                 QWidget*);

        /*
          * Draws just basic keys on the keyboard - just for writing
          */
        void initBasicKeyboard(int offset=0);

        /*
          * Draws nearly all keys of a pc-105 keyboard on the board
          */
        void initExtendedKeyboard();

        /*
          * Deletes all keys for resetting the keyboard
          */
        void resetKeyboard();

        /*
          * Clears lock key and calls clear()
          */
        void clearAnything();

	Plasma::Label* switcher;

    public Q_SLOTS:
        /*
          * Switch key level between lower and upper
          */
        void switchLevel();
        /*
          * Switch between the alternative lvel
          */
        void switchAlternative();
        /*
          * Switch caps key on and off
          */
        void switchCaps();
        /*
          Unsets all pressed keys despite of caps
          */
        void clear();

        /*
          Sets tooltip to a new text
          */
	void setTooltip(QString text, QSizeF buttonSize, QPointF position);
        /*
          * Removes tooltip
          */
        void clearTooltip();

	private:
                QList<AlphaNumKey*> alphaKeys; // normal keys labeled with symbols like a, b, c
                QList<FuncKey*> funcKeys; // functional keys like shift, backspace, enter
                QList<FuncKey*> extKeys; // keys only shown in the extended layout as F1, F2,..
                bool isLevel2; // second key level activated
                bool isAlternative; // alternative key level activated
                bool isLocked; // is lock activddated
                bool basicKeys; // are basic keys displayed
                bool extendedKeys; // are extended keys displayed
                QGraphicsGridLayout *m_layout; // layout the keys are positioned in
		//Plasma::ToolTipContent tooltip;
                QTimer *tooltipTimer;

		Tooltip* tooltip;

};


#endif /* WIDGET_H_ */
