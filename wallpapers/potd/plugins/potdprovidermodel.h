/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QImage>
#include <QTimer>
#include <QUrl>

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

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

    /**
     * Read-only properties that expose data from the provider.
     */
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString localUrl READ localUrl NOTIFY localUrlChanged)
    /**
     * @return The website URL of the image
     */
    Q_PROPERTY(QUrl infoUrl READ infoUrl NOTIFY infoUrlChanged)
    /**
     * @return The remote image URL
     */
    Q_PROPERTY(QUrl remoteUrl READ remoteUrl NOTIFY remoteUrlChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString author READ author NOTIFY authorChanged)

    /**
     * @return the result of the file operation.
     */
    Q_PROPERTY(FileOperationStatus saveStatus MEMBER m_saveStatus NOTIFY saveStatusChanged)

    /**
     * @return the status message after a save operation.
     */
    Q_PROPERTY(QString saveStatusMessage MEMBER m_saveStatusMessage CONSTANT)

    /**
     * @return the folder path of the saved image file.
     */
    Q_PROPERTY(QUrl savedFolder MEMBER m_savedFolder CONSTANT)

    /**
     * @return the path of the saved image file.
     */
    Q_PROPERTY(QUrl savedUrl MEMBER m_savedUrl CONSTANT)

public:
    enum class FileOperationStatus {
        None,
        Successful,
        Failed,
    };
    Q_ENUM(FileOperationStatus)

    enum Roles {
        Id = Qt::UserRole + 1,
    };

    explicit PotdProviderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadPluginMetaData();
    int currentIndex() const;

    bool running() const;
    void setRunning(bool flag);

    QString identifier() const;
    void setIdentifier(const QString &identifier);

    QVariantList arguments() const;
    void setArguments(const QVariantList &args);

    QImage image() const;
    bool loading() const;
    QString localUrl() const;
    QUrl infoUrl() const;
    QUrl remoteUrl() const;
    QString title() const;
    QString author() const;

    /**
     * Opens a Save dialog to choose the save location, and copies the source file to the
     * selected destination.
     */
    Q_INVOKABLE void saveImage();

Q_SIGNALS:
    void currentIndexChanged();
    void runningChanged();
    void identifierChanged();
    void argumentsChanged();

    void imageChanged();
    void loadingChanged();
    void localUrlChanged();
    void infoUrlChanged();
    void remoteUrlChanged();
    void titleChanged();
    void authorChanged();

    void saveStatusChanged();

private Q_SLOTS:
    void slotFinished(PotdProvider *);
    void slotCachingFinished(const QString &source, const PotdProviderData &data);
    void slotError(PotdProvider *);
    void slotPrepareForSleep(bool sleep);

    bool forceUpdateSource();

private:
    void resetData();
    bool updateSource(bool refresh = false);

    void setImage(const QImage &image);
    void setLoading(bool status);
    void setLocalUrl(const QString &urlString);
    void setInfoUrl(const QUrl &url);
    void setRemoteUrl(const QUrl &url);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);

    std::vector<KPluginMetaData> m_providers;
    QString m_identifier;
    int m_currentIndex;
    QVariantList m_args;

    PotdProviderData m_data;
    bool m_loading;

    QTimer m_checkDatesTimer;

    QUrl m_savedFolder;
    QUrl m_savedUrl;
    FileOperationStatus m_saveStatus;
    QString m_saveStatusMessage;
};
