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
#include <QUrl>
#include <KPackage/PackageStructure>

class ComicProviderKross : public ComicProvider
{
        friend class ComicProviderWrapper;
        Q_OBJECT

    public:
        ComicProviderKross(QObject *parent, const QVariantList &args);
        virtual ~ComicProviderKross();

        static KPackage::PackageStructure *packageStructure();

        bool isLeftToRight() const Q_DECL_OVERRIDE;
        bool isTopToBottom() const Q_DECL_OVERRIDE;
        IdentifierType identifierType() const Q_DECL_OVERRIDE;
        QUrl websiteUrl() const Q_DECL_OVERRIDE;
        QUrl shopUrl() const Q_DECL_OVERRIDE;
        QImage image() const Q_DECL_OVERRIDE;
        QString identifier() const Q_DECL_OVERRIDE;
        QString nextIdentifier() const Q_DECL_OVERRIDE;
        QString previousIdentifier() const Q_DECL_OVERRIDE;
        QString firstStripIdentifier() const Q_DECL_OVERRIDE;
        QString stripTitle() const Q_DECL_OVERRIDE;
        QString additionalText() const Q_DECL_OVERRIDE;

    protected:
        void pageRetrieved(int id, const QByteArray &data) Q_DECL_OVERRIDE;
        void pageError(int id, const QString &message) Q_DECL_OVERRIDE;
        void redirected(int id, const QUrl &newUrl) Q_DECL_OVERRIDE;
        QString identifierToString(const QVariant &identifier) const;

    private:
        mutable ComicProviderWrapper m_wrapper;
        static KPackage::PackageStructure *m_packageStructure;
};

#endif
