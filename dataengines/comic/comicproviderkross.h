/*
 *   Copyright (C) 2008 Petri Damstén <damu@iki.fi>
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

#ifndef COMICPROVIDERKROSS_H
#define COMICPROVIDERKROSS_H

#include "comicprovider.h"
#include "comicproviderwrapper.h"

#include <QtGui/QImage>
#include <KUrl>
#include "Plasma/PackageStructure"

class ComicProviderKross : public ComicProvider
{
        friend class ComicProviderWrapper;
        Q_OBJECT

    public:
        ComicProviderKross( QObject *parent, const QVariantList &args );
        virtual ~ComicProviderKross();

        static Plasma::PackageStructure::Ptr packageStructure();

        virtual bool isLeftToRight() const;
        virtual bool isTopToBottom() const;
        virtual IdentifierType identifierType() const;
        virtual KUrl websiteUrl() const;
        virtual KUrl shopUrl() const;
        virtual QImage image() const;
        virtual QString identifier() const;
        virtual QString nextIdentifier() const;
        virtual QString previousIdentifier() const;
        virtual QString firstStripIdentifier() const;
        virtual QString stripTitle() const;
        virtual QString additionalText() const;

    protected:
        virtual void pageRetrieved( int id, const QByteArray &data );
        virtual void pageError( int id, const QString &message );
        QString identifierToString( const QVariant &identifier ) const;

    private:
        mutable ComicProviderWrapper m_wrapper;
        static Plasma::PackageStructure::Ptr m_packageStructure;
};

#endif
