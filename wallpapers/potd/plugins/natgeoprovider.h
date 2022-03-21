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
#include <QRegularExpression>

class KJob;

/**
 * This class provides the image for the National Geographic's photo of the day.
 * Image URL is obtained from https://www.nationalgeographic.com/photo-of-the-day.
 */
class NatGeoProvider : public PotdProvider
{
    Q_OBJECT

public:
    NatGeoProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the APOD provider.
     */
    ~NatGeoProvider() override;

private:
    void pageRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);

private:
    QRegularExpression re;
};

#endif
