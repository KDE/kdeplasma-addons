/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mediaframe.h"
#include "debug_p.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeDatabase>
#include <QRandomGenerator>
#include <QRegularExpression>
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
    qCDebug(PLASMA_MEDIAFRAME) << "Added" << m_filters.count() << "filters";
    // qDebug() << m_filters;

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

int MediaFrame::random(int min, int max)
{
    const auto &[realMin, realMax] = std::minmax(min, max);

    // qDebug() << "random" << min << "<->" << max << "=" << ((qrand()%(max-min+1))+min);
    return QRandomGenerator::global()->bounded(realMin, realMax);
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

void MediaFrame::add(const QString &path)
{
    add(path, AddOption::NON_RECURSIVE);
}

void MediaFrame::add(const QString &path, AddOption option)
{
    if (isAdded(path)) {
        qCWarning(PLASMA_MEDIAFRAME) << "Path" << path << "already added";
        return;
    }

    QUrl url = QUrl(path);
    QString localPath = url.toString(QUrl::PreferLocalFile);
    // qDebug() << "Local path" << localPath << "Path" << path;

    QStringList paths;
    QString filePath;

    if (isDir(localPath)) {
        if (!isDirEmpty(localPath)) {
            QDirIterator dirIterator(
                localPath,
                m_filters,
                QDir::Files,
                (option == AddOption::RECURSIVE ? QDirIterator::Subdirectories | QDirIterator::FollowSymlinks : QDirIterator::NoIteratorFlags));

            while (dirIterator.hasNext()) {
                dirIterator.next();

                filePath = QUrl::fromLocalFile(dirIterator.filePath()).toString();
                paths.append(filePath);
                m_allFiles.append(filePath);
                // qCDebug(PLASMA_MEDIAFRAME) << "Appended" << filePath;
                Q_EMIT countChanged();
            }
            if (paths.count() > 0) {
                m_pathMap.insert(path, paths);
                qCDebug(PLASMA_MEDIAFRAME) << "Added" << paths.count() << "files from" << path;
            } else {
                qCWarning(PLASMA_MEDIAFRAME) << "No images found in directory" << path;
            }
        } else {
            qCWarning(PLASMA_MEDIAFRAME) << "Not adding empty directory" << path;
        }

        // the pictures have to be sorted before adding them to the list,
        // because the QDirIterator sorts them in a different way than QDir::entryList
        // paths.sort();

    } else if (isFile(localPath)) {
        paths.append(path);
        m_pathMap.insert(path, paths);
        m_allFiles.append(path);
        qCDebug(PLASMA_MEDIAFRAME) << "Added" << paths.count() << "files from" << path;
        Q_EMIT countChanged();
    } else {
        if (url.isValid() && !url.isLocalFile()) {
            qCDebug(PLASMA_MEDIAFRAME) << "Adding" << url.toString() << "as remote file";
            paths.append(path);
            m_pathMap.insert(path, paths);
            m_allFiles.append(path);
            Q_EMIT countChanged();
        } else {
            qCWarning(PLASMA_MEDIAFRAME) << "Path" << path << "is not a valid file url or directory";
        }
    }
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
            qCDebug(PLASMA_MEDIAFRAME) << "Nothing in watch list";
        }

        // qDebug() << "watching" << localPath << "for changes";
        m_watcher.addPath(localPath);
        m_watchFile = localPath;
    } else {
        qCWarning(PLASMA_MEDIAFRAME) << "Can't watch remote file" << path << "for changes";
    }
}

bool MediaFrame::isAdded(const QString &path)
{
    return (m_pathMap.contains(path));
}

void MediaFrame::get(QJSValue successCallback)
{
    get(successCallback, QJSValue::UndefinedValue);
}

void MediaFrame::get(QJSValue successCallback, QJSValue errorCallback)
{
    const auto size = static_cast<int>(m_allFiles.count());

    QString errorMessage;
    QJSValueList args;

    if (size == 0) {
        errorMessage = QStringLiteral("No files available");
        qCWarning(PLASMA_MEDIAFRAME) << errorMessage;

        args << QJSValue(errorMessage);
        errorCallback.call(args);
        return;
    }

    if (size == 1) {
        m_current = 0;
    } else if (m_random) {
        if (m_current == -1) {
            m_current = this->random(0, size);
        } else {
            // Make sure we don't select the same image twice in a row by excluding the current index.
            const int next = this->random(0, size - 1);
            m_current = next >= m_current ? next + 1 : next;
        }
    } else {
        if (m_current == -1) {
            m_current = 0;
        } else if (++m_current >= size) {
            qCDebug(PLASMA_MEDIAFRAME) << "Resetting next count from" << m_current << "due to queue size" << size;
            m_current = 0;
        }
    }

    qCDebug(PLASMA_MEDIAFRAME) << "Selected image #" << m_current;

    const QString path{m_allFiles.at(m_current)};

    if (const QUrl url{path}; url.isValid()) {
        if (!url.isLocalFile()) {
            m_filename = path.section(QLatin1Char('/'), -1);

            QString cachedFile = getCacheDirectory() + QLatin1Char('/') + hash(path) + QLatin1Char('_') + m_filename;

            if (isFile(cachedFile)) {
                // File has been cached
                qCDebug(PLASMA_MEDIAFRAME) << path << "is cached as" << cachedFile;

                if (successCallback.isCallable()) {
                    args << QJSValue(cachedFile);
                    successCallback.call(args);
                }
                return;
            }

            m_successCallback = successCallback;
            m_errorCallback = errorCallback;
            m_filename = cachedFile;

            qCDebug(PLASMA_MEDIAFRAME) << path << "doesn't exist locally, trying remote.";

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
        qCCritical(PLASMA_MEDIAFRAME) << errorMessage;

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
        qCCritical(PLASMA_MEDIAFRAME) << errorMessage;

        if (m_errorCallback.isCallable()) {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    } else if (KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob *>(job)) {
        QImage image;

        // TODO make proper caching calls
        QString path = m_filename;
        qCDebug(PLASMA_MEDIAFRAME) << "Saving download to" << path;

        image.loadFromData(transferJob->data());
        image.save(path);

        qCDebug(PLASMA_MEDIAFRAME) << "Saved to" << path;

        if (m_successCallback.isCallable()) {
            args << QJSValue(path);
            m_successCallback.call(args);
        }
    } else {
        errorMessage = QStringLiteral("Unknown error occurred");

        qCCritical(PLASMA_MEDIAFRAME) << errorMessage;

        if (m_errorCallback.isCallable()) {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    }
}
