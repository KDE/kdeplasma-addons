// SPDX-FileCopyrightText: 2020 Guo Yunhe <i@guoyunhe.me>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PIXABAYPROVIDER_H
#define PIXABAYPROVIDER_H

#include "potdprovider.h"

#include <QImage>
#include <QXmlStreamReader>

class KJob;

/**
* This class grabs a random image from the pixabay
* interestingness stream of pictures, for the given date.
* Should there be no image for the current date, it tries
* to grab one from the day before yesterday.
 */
class PixabayProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new pixabay provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        explicit PixabayProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the pixabay provider.
         */
        ~PixabayProvider() override;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        QImage image() const override;

    private:
        void keyRequestFinished(KJob *job);
        void xmlRequestFinished(KJob *job);
        void imageRequestFinished(KJob *job);

    private:
        QString mApiKey;
        QString mImageType;
        QString mOrientation;
        QString mCategory;
        QImage mImage;

        QXmlStreamReader xml;

        int mFailureNumber = 0;

        QStringList m_photoList;
};

#endif
