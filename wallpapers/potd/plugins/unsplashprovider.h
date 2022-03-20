/*
 *   SPDX-FileCopyrightText: 2019 Guo Yunhe <i@guoyunhe.me>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UNSPLASHPROVIDER_H
#define UNSPLASHPROVIDER_H

#include "potdprovider.h"

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
    UnsplashProvider(QObject *parent, const QVariantList &args);

    /**
     * Destroys the Unsplash provider.
     */
    ~UnsplashProvider() override;

private:
    void imageRequestFinished(KJob *job);
};

#endif
