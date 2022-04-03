// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <KIO/Job>
#include <KPluginMetaData>

#include "plasma_potd_export.h"

class QDate;

/**
 * This class is used to store wallpaper data.
 */
struct PotdProviderData {
    QImage wallpaperImage;
    QString wallpaperLocalUrl;
    QUrl wallpaperRemoteUrl;
    QUrl wallpaperInfoUrl;
    QString wallpaperTitle;
    QString wallpaperAuthor;
};
Q_DECLARE_METATYPE(PotdProviderData)

/**
 * This class is an interface for PoTD providers.
 */
class PLASMA_POTD_EXPORT PotdProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new PoTD provider.
     *
     * @param parent The parent object.
     * @param data The metadata of the plugin
     * @param args The arguments.
     */
    explicit PotdProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * @deprecated Since 5.25. The constructor will be removed in Plasma 6.
     */
    explicit PotdProvider(QObject *parent, const QVariantList &args);

    /**
     * Destroys the PoTD provider.
     */
    ~PotdProvider() override;

    /**
     * Returns the requested image.
     *
     * Note: This method returns only a valid image after the
     *       finished() signal has been emitted.
     */
    virtual QImage image() const;

    /**
     * Returns the identifier of the PoTD request (name + date).
     */
    virtual QString identifier() const;

    /**
     * Returns the remote URL of the image from the provider
     *
     * @note No @c virtual to keep binary compatibility.
     * @return the remote URL of the image, if any
     * @since 5.25
     */
    QUrl remoteUrl() const;

    /**
     * Returns the information URL of the image from the provider
     *
     * @return the information URL of the image, if any
     * @since 5.25
     */
    QUrl infoUrl() const;

    /**
     * Returns the title of the image from the provider, if any.
     *
     * @return the title of the image, if any
     * @since 5.25
     */
    QString title() const;

    /**
     * Returns the author of the image from the provider
     *
     * @return the title of the image, if any
     * @since 5.25
     */
    QString author() const;

    /**
     * @return the name of this provider (equiv to X-KDE-PlasmaPoTDProvider-Identifier)
     */
    QString name() const;

    /**
     * @return the date to load for this item, if any
     */
    QDate date() const;

    /**
     * @return if the date is fixed, or if it should always be "today"
     */
    bool isFixedDate() const;

    void refreshConfig();
    void loadConfig();

Q_SIGNALS:
    /**
     * This signal is emitted whenever a request has been finished
     * successfully.
     *
     * @param provider The provider which emitted the signal.
     */
    void finished(PotdProvider *provider);

    /**
     * This signal is emitted whenever an error has occurred.
     *
     * @param provider The provider which emitted the signal.
     */
    void error(PotdProvider *provider);

    void configLoaded(QString apiKey, QString apiSecret);

protected:
    PotdProviderData *potdProviderData() const;

private:
    void configRequestFinished(KJob *job);
    void configWriteFinished(KJob *job);

    const QScopedPointer<class PotdProviderPrivate> d;

    QUrl configRemoteUrl;
    QUrl configLocalUrl;
    QString configLocalPath;
    bool refreshed = false;
};
