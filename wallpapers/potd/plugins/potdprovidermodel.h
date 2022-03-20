/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POTD_PROVIDERMODEL_H
#define POTD_PROVIDERMODEL_H

#include <QAbstractListModel>
#include <QImage>
#include <QTimer>

#include <KPluginMetaData>

#include "potdprovider.h"

/**
 * This class provides the backend of Pictures of The Day from various online
 * websites.
 */
class PotdProviderModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @returns @c true if the update timer is running, @c false otherwise.
     */
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QVariantList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)

    /**
     * Read-only properties that expose data from the provider.
     */
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)
    Q_PROPERTY(QString localUrl READ localUrl NOTIFY localUrlChanged)

public:
    enum Roles {
        Id = Qt::UserRole + 1,
    };

    explicit PotdProviderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadPluginMetaData();
    KPluginMetaData metadata(int index) const;
    Q_INVOKABLE int indexOfProvider(const QString &identifier) const;

    bool running() const;
    void setRunning(bool flag);

    QString identifier() const;
    void setIdentifier(const QString &identifier);

    QVariantList arguments() const;
    void setArguments(const QVariantList &args);

    QImage image() const;
    QString localUrl() const;

Q_SIGNALS:
    void runningChanged();
    void identifierChanged();
    void argumentsChanged();

    void imageChanged();
    void localUrlChanged();

private Q_SLOTS:
    void slotFinished(PotdProvider *);
    void slotCachingFinished(const QString &source, const QString &path, const QImage &img);
    void slotError(PotdProvider *);
    void slotCheckDayChanged();

    bool forceUpdateSource();

private:
    void resetData();
    bool updateSource(bool refresh = false);

    void setImage(const QImage &image);
    void setLocalUrl(const QString &urlString);

    std::vector<KPluginMetaData> m_providers;
    QString m_identifier;
    int m_currentIndex;
    QVariantList m_args;

    PotdProviderData m_data;

    QTimer m_checkDatesTimer;
};

#endif
