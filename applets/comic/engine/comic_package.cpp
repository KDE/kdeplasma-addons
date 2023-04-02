/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comic_package.h"

#include <KLocalizedString>
#include <KPackage/Package>

ComicPackage::ComicPackage(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

void ComicPackage::initPackage(KPackage::Package *package)
{
    package->addDirectoryDefinition("images", QStringLiteral("images"));
    package->setMimeTypes("images", QStringList{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")});

    package->addDirectoryDefinition("scripts", QStringLiteral("code"));
    package->setMimeTypes("scripts", QStringList{QStringLiteral("text/*")});

    package->addFileDefinition("mainscript", QStringLiteral("code/main"));
    // package->setRequired("mainscript", true); Package::isValid() fails with this because of Kross and different file extensions
    package->setDefaultPackageRoot(QStringLiteral("plasma/comics/"));
}

K_PLUGIN_CLASS_WITH_JSON(ComicPackage, "plasma-packagestructure-comic.json")

#include "comic_package.moc"
