/*
 *   SPDX-FileCopyrightText: 2019 Guo Yunhe <i@guoyunhe.me>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

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
    UnsplashProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

private:
    void imageRequestFinished(KJob *job);
};
