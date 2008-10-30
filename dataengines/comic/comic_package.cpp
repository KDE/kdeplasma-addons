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

#include "comic_package.h"

#include "plasma/applet.h"
#include "plasma/package.h"

ComicPackage::ComicPackage( QObject *parent, QVariantList args )
    : Plasma::PackageStructure( parent, "Comic" )
{
    Q_UNUSED( args )
    addDirectoryDefinition( "images", "images", i18n( "Images" ) );
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    setMimetypes( "images", mimetypes );

    addDirectoryDefinition( "scripts", "code", i18n( "Executable Scripts" ) );
    mimetypes.clear();
    mimetypes << "text/*";
    setMimetypes( "scripts", mimetypes );

    addFileDefinition( "mainscript", "code/main", i18n( "Main Script File" ) );

    setDefaultPackageRoot( "plasma/comics/" );
    setServicePrefix( "plasma-comic-" );
}
