/*
    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "translatorpackage.h"

#include "plasma/applet.h"
#include "plasma/package.h"

TranslatorPackage::TranslatorPackage( QObject *parent, const QVariantList& args )
: Plasma::PackageStructure( parent, QLatin1String( "Plasma/Translator" ) )
{
    Q_UNUSED( args )
    addDirectoryDefinition( "images", QLatin1String( "images" ), i18n( "Images" ) );
    QStringList mimetypes;
    mimetypes << QLatin1String( "image/svg+xml" ) << QLatin1String( "image/png" ) << QLatin1String( "image/jpeg" );
    setMimetypes( "images", mimetypes );
    
    addDirectoryDefinition( "scripts", QLatin1String( "code" ), i18n( "Executable Scripts" ) );
    mimetypes.clear();
    mimetypes << QLatin1String( "text/*" );
    setMimetypes( "scripts", mimetypes );
    
    addFileDefinition( "mainscript", QLatin1String( "code/main" ), i18n( "Main Script File" ) );
    
    setDefaultPackageRoot( QLatin1String( "plasma/translator/" ) );
    setServicePrefix( QLatin1String( "plasma-translator-" ) );
}

