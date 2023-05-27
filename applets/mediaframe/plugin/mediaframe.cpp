/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mediaframe.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeDatabase>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTime>
#include <QUrl>

#include <KIO/StoredTransferJob>

MediaFrame::MediaFrame(QObject *parent)
    : QObject(parent)
{
    const auto imageMimeTypeNames = QImageReader::supportedMimeTypes();
    QMimeDatabase mimeDb;
    for (const auto &imageMimeTypeName : imageMimeTypeNames) {
        const auto mimeType = mimeDb.mimeTypeForName(QLatin1String(imageMimeTypeName));
        m_filters << mimeType.globPatterns();
    }
    qDebug() << "Added" << m_filters.count() << "filters";
    // qDebug() << m_filters;
    m_next = 0;

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &MediaFrame::slotItemChanged);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MediaFrame::slotItemChanged);
}

MediaFrame::~MediaFrame() = default;

int MediaFrame::count() const
{
    return m_allFiles.count();
}

bool MediaFrame::random() const
{
    return m_random;
}

void MediaFrame::setRandom(bool random)
{
    if (random != m_random) {
        m_random = random;
        Q_EMIT randomChanged();
    }
}

QStringList MediaFrame::paths() const
{
    return m_paths;
}

void MediaFrame::setPaths(const QStringList &paths)
{
    if (paths == m_paths) {
        return;
    }

    m_paths = paths;

    clear();
    for (const QString &path : paths) {
        const QString localPath = QUrl(path).toLocalFile();
        if (QFileInfo(localPath).isDir()) {
            if (!isDirEmpty(localPath)) {
                QDirIterator dirIterator(path, m_filters, QDir::Files, (QDirIterator::Subdirectories | QDirIterator::FollowSymlinks));
                QStringList localPaths;
                while (dirIterator.hasNext()) {
                    dirIterator.next();

                    const QString filePath = QStringLiteral("file://") + dirIterator.filePath();
                    m_allFiles.append(filePath);
                    localPaths << filePath;
                }
                if (localPaths.size() > 0) {
                    m_pathMap.insert(path, localPaths);
                } else {
                    qWarning() << "No images found in directory" << path;
                }
            } else {
                qWarning() << "Not adding empty directory" << path;
            }

            // the pictures have to be sorted before adding them to the list,
            // because the QDirIterator sorts them in a different way than QDir::entryList
            // paths.sort();

        } else {
            m_pathMap.insert(path, {path});
            m_allFiles.append(path);
        }
    }

    Q_EMIT countChanged();
    Q_EMIT pathsChanged();
}

int MediaFrame::random(int min, int max)
{
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }

    // qDebug() << "random" << min << "<->" << max << "=" << ((qrand()%(max-min+1))+min);
    return (QRandomGenerator::global()->bounded((max - min + 1)) + min);
}

QString MediaFrame::getCacheDirectory()
{
    return QDir::temp().absolutePath();
}

QString MediaFrame::hash(const QString &str)
{
    return QString::fromLatin1(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
}

bool MediaFrame::isDir(const QString &path)
{
    return QDir(path).exists();
}

bool MediaFrame::isDirEmpty(const QString &path)
{
    return (isDir(path) && QDir(path).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
}

bool MediaFrame::isFile(const QString &path)
{
    // Check if the file exists and is not a directory
    return (QFileInfo::exists(path) && QFileInfo(path).isFile());
}

QStringList MediaFrame::add(const QVariant &data) const
{
    QString path;

    if (data.typeId() == QMetaType::QUrl) {
        const QUrl url = data.toUrl();
        path = url.toString();
        if (!url.isLocalFile() || !url.isValid()) {
            qWarning() << "Path" << path << "is not a valid file url or directory";
            return {};
        }

        if (isAdded(path)) {
            qWarning() << "Path" << path << "already added";
            return {};
        }
    } else if (data.canConvert<QList<QUrl>>()) {
        const auto urls = data.value<QList<QUrl>>();
        Q_ASSERT(!urls.empty());
        QStringList paths;
        for (const QUrl &url : urls) {
            const QStringList _paths = add(url);
            if (!_paths.empty()) {
                paths << _paths;
            }
        }
        return paths;
    } else if (data.typeId() == QMetaType::QByteArray) {
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            return {};
        }
        if (QImage::fromData(data.toByteArray()).save(&tempFile, "png")) {
            tempFile.setAutoRemove(false);
            path = QUrl::fromLocalFile(tempFile.fileName()).toString();
        }
    } else {
        return {};
    }

    return {path};
}

void MediaFrame::clear()
{
    m_pathMap.clear();
    m_allFiles.clear();
    Q_EMIT countChanged();
}

void MediaFrame::watch(const QString &path)
{
    QUrl url = QUrl(path);
    QString localPath = url.toString(QUrl::PreferLocalFile);
    if (isFile(localPath)) {
        if (!m_watchFile.isEmpty()) {
            // qDebug() << "Removing" << m_watchFile << "from watch list";
            m_watcher.removePath(m_watchFile);
        } else {
            qDebug() << "Nothing in watch list";
        }

        // qDebug() << "watching" << localPath << "for changes";
        m_watcher.addPath(localPath);
        m_watchFile = localPath;
    } else {
        qWarning() << "Can't watch remote file" << path << "for changes";
    }
}

bool MediaFrame::isAdded(const QString &path) const
{
    return (m_pathMap.contains(path));
}

void MediaFrame::get(QJSValue successCallback)
{
    get(successCallback, QJSValue::UndefinedValue);
}

void MediaFrame::get(QJSValue successCallback, QJSValue errorCallback)
{
    int size = m_allFiles.count() - 1;

    QString path;
    QString errorMessage;
    QJSValueList args;

    if (size < 1) {
        if (size == 0) {
            path = m_allFiles.at(0);

            if (successCallback.isCallable()) {
                args << QJSValue(path);
                successCallback.call(args);
            }
            return;
        } else {
            errorMessage = QStringLiteral("No files available");
            qWarning() << errorMessage;

            args << QJSValue(errorMessage);
            errorCallback.call(args);
            return;
        }
    }

    if (m_random) {
        path = m_allFiles.at(this->random(0, size));
    } else {
        path = m_allFiles.at(m_next);
        m_next++;
        if (m_next > size) {
            qDebug() << "Resetting next count from" << m_next << "due to queue size" << size;
            m_next = 0;
        }
    }

    QUrl url = QUrl(path);

    if (url.isValid()) {
        QString localPath = url.toString(QUrl::PreferLocalFile);

        if (!isFile(localPath)) {
            m_filename = path.section(QLatin1Char('/'), -1);

            QString cachedFile = getCacheDirectory() + QLatin1Char('/') + hash(path) + QLatin1Char('_') + m_filename;

            if (isFile(cachedFile)) {
                // File has been cached
                qDebug() << path << "is cached as" << cachedFile;

                if (successCallback.isCallable()) {
                    args << QJSValue(cachedFile);
                    successCallback.call(args);
                }
                return;
            }

            m_successCallback = successCallback;
            m_errorCallback = errorCallback;
            m_filename = cachedFile;

            qDebug() << path << "doesn't exist locally, trying remote.";

            KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
            connect(job, &KJob::finished, this, &MediaFrame::slotFinished);

        } else {
            if (successCallback.isCallable()) {
                args << QJSValue(path);
                successCallback.call(args);
            }
            return;
        }
    } else {
        errorMessage = path + QLatin1String(" is not a valid URL");
        qCritical() << errorMessage;

        if (errorCallback.isCallable()) {
            args << QJSValue(errorMessage);
            errorCallback.call(args);
        }
        return;
    }
}

void MediaFrame::pushHistory(const QString &string)
{
    const int oldCount = m_history.count();

    m_history.prepend(string);

    // Keep a sane history size
    if (m_history.length() > 50) {
        m_history.removeLast();
    }

    if (oldCount != m_history.count()) {
        Q_EMIT historyLengthChanged();
    }
}

QString MediaFrame::popHistory()
{
    if (m_history.isEmpty()) {
        return QString();
    }

    const QString item = m_history.takeFirst();
    Q_EMIT historyLengthChanged();
    return item;
}

int MediaFrame::historyLength() const
{
    return m_history.length();
}

void MediaFrame::pushFuture(const QString &string)
{
    m_future.prepend(string);
    Q_EMIT futureLengthChanged();
}

QString MediaFrame::popFuture()
{
    if (m_future.isEmpty()) {
        return QString();
    }

    const QString item = m_future.takeFirst();
    Q_EMIT futureLengthChanged();
    return item;
}

int MediaFrame::futureLength() const
{
    return m_future.length();
}

void MediaFrame::slotItemChanged(const QString &path)
{
    Q_EMIT itemChanged(path);
}

void MediaFrame::slotFinished(KJob *job)
{
    QString errorMessage;
    QJSValueList args;

    if (job->error()) {
        errorMessage = QLatin1String("Error loading image: ") + job->errorString();
        qCritical() << errorMessage;

        if (m_errorCallback.isCallable()) {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    } else if (KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob *>(job)) {
        QImage image;

        // TODO make proper caching calls
        QString path = m_filename;
        qDebug() << "Saving download to" << path;

        image.loadFromData(transferJob->data());
        image.save(path);

        qDebug() << "Saved to" << path;

        if (m_successCallback.isCallable()) {
            args << QJSValue(path);
            m_successCallback.call(args);
        }
    } else {
        errorMessage = QStringLiteral("Unknown error occurred");

        qCritical() << errorMessage;

        if (m_errorCallback.isCallable()) {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    }
}
