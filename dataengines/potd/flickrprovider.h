// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
// SPDX-FileCopyrightText: 2008 Georges Toth <gtoth@trypill.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FLICKRPROVIDER_H
#define FLICKRPROVIDER_H

#include "potdprovider.h"

#include <QDate>
#include <QXmlStreamReader>

#include <KIO/Job>

/**
 * This class grabs a random image from the flickr
 * interestingness stream of pictures, for the given date.
 * Should there be no image for the current date, it tries
 * to grab one from the day before yesterday.
 */
class FlickrProvider : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new flickr provider.
     *
     * @param parent The parent object.
     * @param args The arguments.
     */
    explicit FlickrProvider(QObject *parent, const QVariantList &args);

    /**
     * Destroys the flickr provider.
     */
    ~FlickrProvider() override;

private:
    void sendXmlRequest(const QString &apiKey, const QString &apiSecret);
    void xmlRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);

private:
    QDate mActualDate;
    QString mApiKey;

    QXmlStreamReader xml;

    int mFailureNumber = 0;

    QStringList m_photoList;
};

#endif
