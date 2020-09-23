/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NOAAPROVIDER_H
#define NOAAPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides the image for NOAA Environmental Visualization Laboratory
 * Image Of the Day
 * located at https://www.nesdis.noaa.gov/content/imagery-and-data.
 */
class NOAAProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new NOAA provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        NOAAProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the NOAA provider.
         */
        ~NOAAProvider() override;

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
