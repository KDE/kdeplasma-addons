/*
 *   SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BINGPROVIDER_H
#define BINGPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides the image for the Bing's homepage
 * url is obtained from https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1
 */
class BingProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new Bing provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        BingProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the Bing provider.
         */
        ~BingProvider() override;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        QImage image() const override;

    private:
        void pageRequestFinished(KJob *job);
        void imageRequestFinished(KJob *job);

    private:
        QImage mImage;
};

#endif
