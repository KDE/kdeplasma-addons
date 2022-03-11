/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef COMIC_ENGINE_H
#define COMIC_ENGINE_H

// Qt
#include <QIcon>
#include <QNetworkConfigurationManager>
#include <QSet>
#include <QVariant>

class ComicProvider;

struct ComicProviderInfo {
    QString pluginId;
    QString name;
    QString icon;
};

/**
 * This class provides the comic strip.
 *
 * The query keys have the following structure:
 *   \<comic_identifier\>:\<suffix\>
 * usually the suffix is the date
 * e.g.
 *   userfriendly:2007-07-19
 * but some other comics uses numerical identifiers, like
 *   xkcd:378
 * if the suffix is empty the latest comic will be returned
 *
 */
class ComicEngine : public QObject
{
    Q_OBJECT

public:
    ComicEngine();
    ~ComicEngine() override;

    QList<ComicProviderInfo> loadProviders();

    void setMaxComicLimit(int maxComicLimit);
    using ComicRequestCallback = const std::function<void(const QVariantMap &data)> &;
    bool requestSource(const QString &identifier, ComicRequestCallback callback);

Q_SIGNALS:
    void sourceUpdated(const QString &identifier);

protected:
    void init();

private:
    void finished(ComicProvider *, ComicRequestCallback callback);
    void error(ComicProvider *, ComicRequestCallback callback);
    void onOnlineStateChanged(bool);
    void setComicData(ComicProvider *provider, ComicRequestCallback callback);

private:
    bool mEmptySuffix;
    QString lastCachedIdentifier(const QString &identifier) const;
    QString mIdentifierError;
    QHash<QString, ComicProvider *> m_jobs;
    QNetworkConfigurationManager m_networkConfigurationManager;
    QSet<QString> mProviders;
};

#endif
