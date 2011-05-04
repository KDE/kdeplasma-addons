/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef CHECK_NEW_STRIPS_H
#define CHECK_NEW_STRIPS_H

#include <Plasma/DataEngine>

/**
 * This class searches for the newest comic strips of predefined comics in a defined intervall.
 * Once found it emits lastStrip
 */
class CheckNewStrips : public QObject
{
    Q_OBJECT

    public:
        CheckNewStrips( const QStringList &identifiers, Plasma::DataEngine *engine, int minutes, QObject *parent = 0 );

    signals:
        /**
         * @param index of the identifier in identifiers
         * @param identifier of the comic
         * @param suffix of the last comic strip
         * @see CheckNewStrips
         */
        void lastStrip( int index, const QString &identifier, const QString &suffix );

    public slots:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );

    private slots:
        void start();

    private:
        int mMinutes;
        int mIndex;
        Plasma::DataEngine *mEngine;
        const QStringList mIdentifiers;
};

#endif
