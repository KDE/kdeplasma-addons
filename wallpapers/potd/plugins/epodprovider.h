/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EPODPROVIDER_H
#define EPODPROVIDER_H

#include "potdprovider.h"

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
    EpodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the EPOD provider.
     */
    ~EpodProvider() override;

private:
    void pageRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};

#endif
