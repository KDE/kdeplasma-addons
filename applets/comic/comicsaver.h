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

#ifndef COMIC_SAVER_H
#define COMIC_SAVER_H

class ComicData;
class SavingDir;

/**
 * ComicSaver takes care of saving a comic strip to a user chosen
 * destination.
 * Further if available Nepomuk is used to store the title, author
 * etc.
 */
class ComicSaver
{
    public:
        ComicSaver(SavingDir *savingDir);

        /**
         * Asks the user for a destination to save the specified
         * comic to. If possible writes it to that destination.
         * @param comic the comic to save
         * @return true if saving worked, false if there was a problem
         */
        bool save(const ComicData &comic);

    private:
        SavingDir *mSavingDir;
};

#endif
