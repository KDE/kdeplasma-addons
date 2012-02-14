/***************************************************************************
 *   Copyright (C) 2011-2012 Matthias Fuchs <mat69@gmx.net>                *
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

#ifndef COMIC_TYPE_H
#define COMIC_TYPE_H

class KConfigGroup;
class QString;

enum IdentifierType {
    Date = 0,
    Number,
    String
};

/**
 * Provides access (read/write) to the directory that should be used
 * whenever the user is presented with a file selection dialog.
 */
class SavingDir
{
    public:
        /**
         * @param config the config that should be used to retrieve
         * the saving directory and to store it to in case of changes
         */
        SavingDir(const KConfigGroup &config);

        ~SavingDir();

        /**
         * @return the directory to be displayed to the user
         */
        QString getDir() const;

        /**
         * Set the directory that should be displayed to the user first
         * when choosing a destination. Automatically writes the directory
         * to the config, if one was specified in init.
         * @param dir the directory to display the user first
         * @see init
         */
        void setDir(const QString &dir);

    private:
        class SavingDirPrivate;
        SavingDirPrivate *d;
};

#endif
