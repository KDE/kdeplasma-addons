/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef APODPROVIDER_H
#define APODPROVIDER_H

#include "potdprovider.h"

#include <QImage>

class KJob;

/**
 * This class provides the image for APOD
 * "Astronomy Picture Of the Day"
 * located at https://antwrp.gsfc.nasa.gov/apod.
 * Direct link to the picture of the day page is
 * https://antwrp.gsfc.nasa.gov/apod/apYYMMDD.html
 * where YY is the year last 2 digits,
 * MM is the month and DD the day, in 2 digits.
 */
class ApodProvider : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new APOD provider.
     *
     * @param parent The parent object.
     * @param args The arguments.
     */
    explicit ApodProvider(QObject *parent, const QVariantList &args);

    /**
     * Destroys the APOD provider.
     */
    ~ApodProvider() override;

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
