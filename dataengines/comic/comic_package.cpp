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

#include <KPackage/Package>
#include <KLocalizedString>
#include <QDebug>

ComicPackage::ComicPackage(QObject *parent, const QVariantList& args)
    : KPackage::PackageStructure(parent, args)
{
}

void ComicPackage::initPackage(KPackage::Package *package)
{
    QStringList mimetypes;
    package->addDirectoryDefinition("images", QLatin1String("images"), i18n("Images"));
    mimetypes << QLatin1String("image/svg+xml") << QLatin1String("image/png") << QLatin1String("image/jpeg");
    package->setMimeTypes("images", mimetypes);

    mimetypes.clear();
    package->addDirectoryDefinition("scripts", QLatin1String("code"), i18n("Executable Scripts"));
    mimetypes << QLatin1String("text/*");
    package->setMimeTypes("scripts", mimetypes);

    package->addFileDefinition("mainscript", QLatin1String("code/main"), i18n("Main Script File"));
    //package->setRequired("mainscript", true); Package::isValid() fails with this because of Kross and different file extensions
    package->setDefaultPackageRoot(QStringLiteral("plasma/comics/"));
}

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(ComicPackage, "plasma-packagestructure-comic.json")

#include "comic_package.moc"
