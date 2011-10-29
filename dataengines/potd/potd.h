/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.   
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

#ifndef POTD_DATAENGINE_H
#define POTD_DATAENGINE_H

#include "plasma/dataengine.h"

class PotdProvider;

class QTimer;

/**
 * This class provides the Pictures of The Day from various online websites.
 *
 * The query keys have the following structure:
 *   <potd_identifier>:<date>
 * e.g.
 *   apod:2007-07-19
 *
 */
class PotdEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        PotdEngine( QObject* parent, const QVariantList& args );
        ~PotdEngine();

    protected:
        void init();
        bool sourceRequestEvent( const QString &identifier );

    protected Q_SLOTS:
        bool updateSourceEvent( const QString &identifier );

    private Q_SLOTS:
        void finished( PotdProvider* );
        void error( PotdProvider* );
        void checkDayChanged();
        void cachingFinished( const QString &source, const QString &path, const QImage &img );

    private:
        bool updateSource( const QString &identifier, bool loadCachedAlways );

        QMap<QString, KService::Ptr> mFactories;
        QTimer *m_checkDatesTimer;
        bool m_canDiscardCache;
};

K_EXPORT_PLASMA_DATAENGINE(potd, PotdEngine)

#endif
