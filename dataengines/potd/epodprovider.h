/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EPODPROVIDER_H
#define EPODPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides the image for EPOD
 * "Earth Science Picture Of the Day"
 * located at https://epod.usra.edu/.
 */
class EpodProvider : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new EPOD provider.
     *
     * @param parent The parent object.
     * @param args The arguments.
     */
    EpodProvider(QObject *parent, const QVariantList &args);

    /**
     * Destroys the EPOD provider.
     */
    ~EpodProvider() override;

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
