/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "potdprovider.h"

class KJob;

/**
 * This class provides the image for APOD
 * "Astronomy Picture Of the Day"
 * located at https://science.nasa.gov/apod/.
 * Direct link to the picture of the day page is
 * found inside the `<apod:hdurl>` element inside
 * the rss feed located at:
 * https://science.nasa.gov/feed/apod-basic/
 */
class ApodProvider : public PotdProvider
{
    Q_OBJECT

public:
    explicit ApodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

private:
    void feedRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};
