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

#include "comicproviderkross.h"
#include "comic_package.h"

COMICPROVIDER_EXPORT_PLUGIN( ComicProviderKross, "ComicProviderKross", "" )

Plasma::PackageStructure::Ptr ComicProviderKross::m_packageStructure( 0 );

ComicProviderKross::ComicProviderKross( QObject *parent, const QVariantList &args )
: ComicProvider( parent, args )
, m_wrapper( this )
{
}

ComicProviderKross::~ComicProviderKross()
{
}

ComicProvider::IdentifierType ComicProviderKross::identifierType() const
{
    return m_wrapper.identifierType();
}

KUrl ComicProviderKross::websiteUrl() const
{
    return m_wrapper.websiteUrl();
}

QImage ComicProviderKross::image() const
{
    return m_wrapper.image();
}

QString ComicProviderKross::identifier() const
{
    return m_wrapper.identifier();
}

QString ComicProviderKross::nextIdentifier() const
{
    QString result = m_wrapper.nextIdentifier();
    if ( !m_wrapper.functionCalled() ) {
        return ComicProvider::nextIdentifier();
    }
    return result;
}

QString ComicProviderKross::previousIdentifier() const
{
    QString result = m_wrapper.previousIdentifier();
    if ( !m_wrapper.functionCalled() ) {
        return ComicProvider::previousIdentifier();
    }
    return result;
}

QString ComicProviderKross::firstStripIdentifier() const
{
    QString result = m_wrapper.firstStripIdentifier();
    if ( !m_wrapper.functionCalled() ) {
        return ComicProvider::firstStripIdentifier();
    }
    return result;
}

QString ComicProviderKross::stripTitle() const
{
    return m_wrapper.stripTitle();
}

QString ComicProviderKross::additionalText() const
{
    return m_wrapper.additionalText();
}

void ComicProviderKross::pageRetrieved( int id, const QByteArray &data )
{
    m_wrapper.pageRetrieved( id, data );
}

void ComicProviderKross::pageError( int id, const QString &message )
{
    m_wrapper.pageError( id, message );
}

Plasma::PackageStructure::Ptr ComicProviderKross::packageStructure()
{
    if ( !m_packageStructure ) {
        m_packageStructure = new ComicPackage();
    }
    return m_packageStructure;
}

#include "comicproviderkross.moc"
