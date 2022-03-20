/*
 *   SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BINGPROVIDER_H
#define BINGPROVIDER_H

#include "potdprovider.h"

class KJob;

/**
 * This class provides the image for the Bing's homepage
 * url is obtained from https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1
 */
class BingProvider : public PotdProvider
{
    Q_OBJECT

public:
    BingProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the Bing provider.
     */
    ~BingProvider() override;

private:
    void pageRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};

#endif
