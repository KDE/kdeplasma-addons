/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2013 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NATGEOPROVIDER_H
#define NATGEOPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>
#include <QRegularExpression>

class KJob;

/**
 * This class provides the image for APOD 
 * "Astronomy Picture Of the Day"
 * located at http://antwrp.gsfc.nasa.gov/apod.
 * Direct link to the picture of the day page is 
 * http://antwrp.gsfc.nasa.gov/apod/apYYMMDD.html 
 * where YY is the year last 2 digits,
 * MM is the month and DD the day, in 2 digits.
 */
class NatGeoProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new APOD provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        NatGeoProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the APOD provider.
         */
        ~NatGeoProvider() override;

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

        QRegularExpression re;
};

#endif
