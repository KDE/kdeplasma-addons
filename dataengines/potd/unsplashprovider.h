/*
 *   Copyright 2019 Guo Yunhe <i@guoyunhe.me>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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

#ifndef UNSPLASHPROVIDER_H
#define UNSPLASHPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides random wallpapers from Unsplash Wallpapers
 * https://unsplash.com/wallpaper/
 */
class UnsplashProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new Unsplash provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        UnsplashProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the Unsplash provider.
         */
        ~UnsplashProvider() override;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        QImage image() const override;

    private:
        void imageRequestFinished(KJob *job);

    private:
        QImage mImage;
};

#endif
