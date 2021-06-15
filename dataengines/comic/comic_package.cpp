/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comic_package.h"

#include <KLocalizedString>
#include <KPackage/Package>
#include <QDebug>

ComicPackage::ComicPackage(QObject *parent, const QVariantList &args)
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
    // package->setRequired("mainscript", true); Package::isValid() fails with this because of Kross and different file extensions
    package->setDefaultPackageRoot(QStringLiteral("plasma/comics/"));
}

K_PLUGIN_CLASS_WITH_JSON(ComicPackage, "plasma-packagestructure-comic.json")

#include "comic_package.moc"
