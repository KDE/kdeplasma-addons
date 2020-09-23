/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef COMIC_PACKAGE_H
#define COMIC_PACKAGE_H

#include <KPackage/PackageStructure>

class ComicPackage : public KPackage::PackageStructure
{
    Q_OBJECT

public:
        explicit ComicPackage(QObject *parent = nullptr, const QVariantList& args = QVariantList());
        void initPackage(KPackage::Package *package) override;
};

#endif
