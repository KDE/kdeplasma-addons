/***************************************************************************
 *   Copyright (C) 2012 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef STRIP_SELECTOR_H
#define STRIP_SELECTOR_H

#include <QtCore/QObject>

#include "comicinfo.h"

class ComicData;
class QDate;

/**
 * Enables users to visually select a strip they want to navigate to.
 * Subclasses implement different Selectors for the different comic
 * types.
 * @note use the StripSelectorFactory to retrieve an appropriate
 * StripSelector
 */
class StripSelector : public QObject
{
    Q_OBJECT

    public:
        virtual ~StripSelector();

        /**
         * Select a strip depending on the subclass
         * @param currentStrip the currently active strip
         * @note StripSelector takes care to delete itself
         */
        virtual void select(const ComicData &currentStrip) = 0;

    signals:
        /**
         * @param strip the selected strip, can be empty
         * 
         */
        void stripChosen(const QString &strip);

    protected:
        explicit StripSelector(QObject *parent = 0);
};

/**
 * Class to retrrieve the correct StripSelector depending on the
 * specified IdentifierType
 */
class StripSelectorFactory
{
    public:
        static StripSelector *create(IdentifierType type);
};

#endif
