/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef COMIC_DATAENGINE_H
#define COMIC_DATAENGINE_H

#include "plasma/dataengine.h"

class ComicProvider;

/**
 * This class provides the comic strip.
 *
 * The query keys have the following structure:
 *   <comic_identifier>:<suffix>
 * usually the suffix is the date
 * e.g.
 *   userfriendly:2007-07-19
 * but some other comics uses numerical identifiers, like
 *   xkcd:378
 * if the suffix is empty the latest comic will be returned
 *
 */
class ComicEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        ComicEngine( QObject* parent, const QVariantList& args );
        ~ComicEngine();

    protected:
        void init();
        bool sourceRequested( const QString &identifier );

    protected Q_SLOTS:
        bool updateSource( const QString &identifier );

    private Q_SLOTS:
        void finished( ComicProvider* );
        void error( ComicProvider* );

    private:
        bool mEmptySuffix;
        QMap<QString, KService::Ptr> mFactories;
};

K_EXPORT_PLASMA_DATAENGINE(comic, ComicEngine)

#endif
