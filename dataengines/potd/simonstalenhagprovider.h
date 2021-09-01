/*
 *   SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#ifndef UNSPLASHPROVIDER_H
#define UNSPLASHPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides random wallpapers from Simon Stalenhag website
 * http://simonstalenhag.se/
 * see also: https://github.com/a-andreyev/simonstalenhag-se-metadata
 */
class SimonStalenhagProvider : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new SimonStalenhag provider.
     *
     * @param parent The parent object.
     * @param args The arguments.
     */
    SimonStalenhagProvider(QObject *parent, const QVariantList &args);

    /**
     * Returns the requested image.
     *
     * Note: This method returns only a valid image after the
     *       finished() signal has been emitted.
     */
    QImage image() const override;

private:
    void entrypointRequestFinished(KJob *job);
    void metaDataRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);

private:
    QImage mImage;
};

#endif
