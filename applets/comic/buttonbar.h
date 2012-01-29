/***************************************************************************
 *   Copyright (C) 2008-2012 Matthias Fuchs <mat69@gmx.net>                *
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

#ifndef BUTTON_BAR_H
#define BUTTON_BAR_H

#include <QtCore/QObject>
#include <QtCore/QSizeF>

class QPointF;
class QPropertyAnimation;

namespace Plasma {
    class Frame;
    class PushButton;
}

/**
 * ButtonBar is a container holding a next and previous button
 * as well as a zoom button.
 * It utilizes a Plasma::Frame to hold all the content
 */
class ButtonBar : public QObject
{
    Q_OBJECT

    public:
        /**
         * Creates a ButtonBar
         * @note the button bar is hidden by default and has no
         * position associated with it
         */
        explicit ButtonBar(QObject *parent);
        ~ButtonBar();

        QSizeF size() const;

        /**
         * Sets the position of the frame
         * @param pos the position the frame should be placed at
         */
        void setPos(const QPointF &pos);

        /**
         * Will fade in the button bar if it was hidden
         * TODO the hidden part
         */
        void show();

        /**
         * Will fade out the button bar if it was visible
         * TODO the visible part
         */
        void hide();

        /**
         * Enables the next button
         * @param enabled true if the button should be enabled
         * @note disabled by default
         */
        void setNextEnabled(bool enabled);

        /**
         * Enables the previous button
         * @param enabled true if the button should be enabled
         * @note disabled by default
         */
        void setPrevEnabled(bool enabled);

    signals:
        void nextClicked();
        void prevClicked();
        void zoomClicked();

    private:
        Plasma::Frame *mFrame;
        QPropertyAnimation *mFrameAnim;
        Plasma::PushButton *mPrev;
        Plasma::PushButton *mNext;
        Plasma::PushButton *mZoom;
};

#endif
