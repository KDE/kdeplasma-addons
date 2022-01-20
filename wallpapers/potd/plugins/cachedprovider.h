/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CACHEDPROVIDER_H
#define CACHEDPROVIDER_H

#include <QImage>
#include <QRunnable>

#include "potdprovider.h"

/**
 * This class provides pictures from the local cache.
 */
class CachedProvider : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new cached provider.
     *
     * @param identifier The identifier of the cached picture.
     * @param parent The parent object.
     */
    CachedProvider(const QString &identifier, QObject *parent);

    /**
     * Destroys the cached provider.
     */
    ~CachedProvider() override;

    /**
     * Returns the identifier of the picture request (name + date).
     */
    QString identifier() const override;

    /**
     * Returns whether a picture with the given @p identifier is cached.
     */
    static bool isCached(const QString &identifier, bool ignoreAge = false);

    /**
     * Returns a path for the given identifier
     */
    static QString identifierToPath(const QString &identifier);

private Q_SLOTS:
    void triggerFinished(const QImage &image);

private:
    QString mIdentifier;
};

class LoadImageThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit LoadImageThread(const QString &filePath);
    void run() override;

Q_SIGNALS:
    void done(const QImage &pixmap);

private:
    QString m_filePath;
};

class SaveImageThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    SaveImageThread(const QString &identifier, const QImage &image);
    void run() override;

Q_SIGNALS:
    void done(const QString &source, const PotdProviderData &data);

private:
    QImage m_image;
    QString m_identifier;
};

#endif
