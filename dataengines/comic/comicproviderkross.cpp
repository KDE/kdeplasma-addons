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
    : ComicProvider( parent, args ), m_wrapper( this )
{
}

ComicProviderKross::~ComicProviderKross()
{
}

bool ComicProviderKross::isLeftToRight() const
{
    return m_wrapper.isLeftToRight();
}

bool ComicProviderKross::isTopToBottom() const
{
    return m_wrapper.isTopToBottom();
}

ComicProvider::IdentifierType ComicProviderKross::identifierType() const
{
    return m_wrapper.identifierType();
}

KUrl ComicProviderKross::websiteUrl() const
{
    return m_wrapper.websiteUrl();
}

KUrl ComicProviderKross::shopUrl() const
{
    return m_wrapper.shopUrl();
}

QImage ComicProviderKross::image() const
{
    return m_wrapper.comicImage();
}

QString ComicProviderKross::identifierToString( const QVariant &identifier ) const
{
    QString result;

    if ( !identifier.isNull() && identifier.type() != QVariant::Bool ) {
        if ( identifierType() == ComicProvider::DateIdentifier ) {
            result = identifier.toDate().toString( Qt::ISODate );
        } else {
            result = identifier.toString();
        }
    }
    return result;
}

QString ComicProviderKross::identifier() const
{
    return pluginName() + ':' + identifierToString( m_wrapper.identifierVariant() );
}

QString ComicProviderKross::nextIdentifier() const
{
    return identifierToString( m_wrapper.nextIdentifierVariant() );
}

QString ComicProviderKross::previousIdentifier() const
{
    return  identifierToString( m_wrapper.previousIdentifierVariant() );
}

QString ComicProviderKross::firstStripIdentifier() const
{
    return identifierToString( m_wrapper.firstIdentifierVariant() );
}

QString ComicProviderKross::stripTitle() const
{
    return m_wrapper.title();
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
