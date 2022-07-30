/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <unordered_map>

#include <QDate>
#include <QObject>
#include <QTimer>

#include <KPluginMetaData>

#include "config-NetworkManagerQt.h"
#include "potdprovider.h"

/**
 * This class provides data for the specific identifier and arguments
 */
class PotdClient : public QObject
{
    Q_OBJECT

public:
    PotdClient(const KPluginMetaData &metadata, const QVariantList &args, QObject *parent = nullptr);

    void updateSource(bool refresh = false);
#if HAVE_NetworkManagerQt
    void setUpdateOverMeteredConnection(int value);
#endif

    KPluginMetaData m_metadata;
    PotdProviderData m_data;
    bool m_loading = false;

Q_SIGNALS:
    void imageChanged();
    void loadingChanged();
    void localUrlChanged();
    void infoUrlChanged();
    void remoteUrlChanged();
    void titleChanged();
    void authorChanged();
    void done(PotdClient *client, bool success);

private Q_SLOTS:
    void slotFinished(PotdProvider *provider);
    void slotError(PotdProvider *provider);
    void slotCachingFinished(const QString &source, const PotdProviderData &data);

private:
    void setImage(const QImage &image);
    void setLoading(bool status);
    void setLocalUrl(const QString &urlString);
    void setInfoUrl(const QUrl &url);
    void setRemoteUrl(const QUrl &url);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);

    QString m_identifier;
    QVariantList m_args;
#if HAVE_NetworkManagerQt
    int m_doesUpdateOverMeteredConnection = 0;
#endif

    friend class PotdEngine;
};

/**
 * This class manages the clients for the Pictures of The Day plugin
 */
class PotdEngine : public QObject
{
    Q_OBJECT

public:
    explicit PotdEngine(QObject *parent = nullptr);

    /**
     * Registers the @p identifier in the engine
     *
     * A client will be created, and will be automatically destroyed when no backend has
     * the identifier.
     *
     * @return the client that relays signals for the specific identifier
     */
    PotdClient *registerClient(const QString &identifier, const QVariantList &args);
    void unregisterClient(const QString &identifier, const QVariantList &args);

    void updateSource(bool refresh);

private Q_SLOTS:
    void forceUpdateSource();
    void slotDone(PotdClient *client, bool success);
    void slotPrepareForSleep(bool sleep);

private:
    void loadPluginMetaData();

    struct ClientPair {
        PotdClient *const client = nullptr;
        int instanceCount = 0;
    };
    std::unordered_multimap<QString /* identifier */, ClientPair> m_clientMap;
    std::unordered_map<QString, KPluginMetaData> m_providersMap;

    QTimer m_checkDatesTimer;
    int m_updateCount = 0;

    bool m_lastUpdateSuccess = false;
};
