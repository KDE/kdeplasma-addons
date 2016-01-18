/*
 *  Copyright 2015  Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

#include "mediaframe.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
#include <QImageReader>
#include <QTime>
#include <QRegularExpression>
#include <QCryptographicHash>

//#include <KUrl>
#include <KIO/StoredTransferJob>
#include <KIO/Job>

MediaFrame::MediaFrame(QObject *parent) : QObject(parent)
{
    qsrand(QTime::currentTime().msec());
    
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    //qDebug() << "List" << list;
    for(int i=0; i<list.count(); ++i){
        QString str(list[i].constData());
        m_filters << "*."+str;
    }
    qDebug() << "Added" << list.count() << "filters";
    //qDebug() << m_filters;
    m_watchFile = "";
    m_next = 0;
    
    QObject::connect(&m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slotItemChanged(QString)));
    QObject::connect(&m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(slotItemChanged(QString)));
}

MediaFrame::~MediaFrame()
{
}

int MediaFrame::count() const
{
    return m_allFiles.count();
}


bool MediaFrame::random() const
{
    return m_random;
}

void MediaFrame::setRandom(const bool &random)
{
    if (random != m_random)
    {
        m_random = random;
        emit randomChanged();
    }
}

int MediaFrame::random(int min, int max)
{
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    
    qDebug() << "random" << min << "<->" << max << "=" << ((qrand()%(max-min+1))+min);
    return ((qrand()%(max-min+1))+min);
}

QString MediaFrame::getCacheDirectory()
{
    return QDir::temp().absolutePath();
}

QString MediaFrame::hash(QString str)
{
    return QString( QCryptographicHash::hash( str.toUtf8(), QCryptographicHash::Md5).toHex() );
}

bool MediaFrame::isDir(const QString &path)
{
    return QDir(path).exists();
}

bool MediaFrame::isDirEmpty(const QString &path)
{
    return (isDir(path) && QDir(path).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0);
}

bool MediaFrame::isFile(const QString &path)
{
    QFileInfo checkFile(path);
    // Check if the file exists and not a directory
    return (checkFile.exists() && checkFile.isFile());
}

void MediaFrame::add(const QString &path)
{
    add(path, false);
}

void MediaFrame::add(const QString &path, bool recursive)
{
    if(has(path))
    {
        qWarning() << "Path" << path << "already exists";
        return;
    }
    
    QUrl url = QUrl(path);
    QString localPath = url.toString(QUrl::PreferLocalFile);
    qDebug() << "Local path" << localPath;
    
    QStringList paths;
    QString filePath;
    
    if(isDir(localPath)) {
        
        if(!isDirEmpty(localPath))
        {
            QDirIterator dirIterator(localPath, m_filters, QDir::Files, (recursive ? QDirIterator::Subdirectories | QDirIterator::FollowSymlinks : QDirIterator::NoIteratorFlags));
            
            while (dirIterator.hasNext()) {
                dirIterator.next();
                
                filePath = dirIterator.filePath();
                paths.append(filePath);
                m_allFiles.append(filePath);
                qDebug() << "Appended" << filePath;
                emit countChanged();
            }
            if(paths.count() > 0)
            {
                m_pathMap[path] = paths;
                qDebug() << "Added" << paths.count() << "files from" << path;
            }
            else
            {
                qWarning() << "No images found in directory" << path;
            }
        }
        else
        {
            qWarning() << "Not adding empty directory" << path;
        }
    
        // the pictures have to be sorted before adding them to the list,
        // because the QDirIterator sorts them in a different way than QDir::entryList
        //paths.sort();
        
    }
    else if(isFile(localPath))
    {
        paths.append(path);
        m_pathMap[path] = paths;
        m_allFiles.append(path);
        qDebug() << "Added" << paths.count() << "files from" << path;
        emit countChanged();
    }
    else
    {
        if (url.isValid() && !url.isLocalFile())
        {
            qDebug() << "Adding" << url.toString() << "as remote file";
            paths.append(path);
            m_pathMap[path] = paths;
            m_allFiles.append(path);
            emit countChanged();
        }
        else
        {
            qWarning() << "Path" << path << "is not a valid file url or directory";
        }
    }
    
}

void MediaFrame::clear()
{
    m_pathMap = QHash<QString, QStringList>();
    m_allFiles = QStringList();
    emit countChanged();
}

void MediaFrame::watch(const QString &path)
{
    QUrl url = QUrl(path);
    QString localPath = url.toString(QUrl::PreferLocalFile);
    if(isFile(localPath))
    {
        if(m_watchFile != "")
        {
            qDebug() << "Removing" << m_watchFile << "from watch list";
            m_watcher.removePath(m_watchFile);
        }
        else
        {
            qDebug() << "Nothing in watch list";
        }
        
        qDebug() << "watching" << localPath << "for changes";
        m_watcher.addPath(localPath);
        m_watchFile = QString(localPath);
    }
    else
    {
        qWarning() << "Can't watch" << path << "for changes";
    }
}

bool MediaFrame::has(const QString &path)
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
    QString errorMessage = QString("");
    QJSValueList args;
    
    if(size < 1)
    {
        if(size == 0)
        {
            path = m_allFiles.at(0);
            
            if(successCallback.isCallable())
            {
                args << QJSValue(path);
                successCallback.call(args);
            }
            return;
        }
        else
        {
            errorMessage = "No files available";
            qWarning() << errorMessage;
            
            args << QJSValue(errorMessage);
            errorCallback.call(args);
            return;
        }
    }
    
    if(m_random)
    {
        path = m_allFiles.at(this->random(0, size));
    }
    else
    {
        path = m_allFiles.at(m_next);
        m_next++;
        if(m_next > size)
        {
            qDebug() << "Resetting next count from" << m_next << "due to queue size" << size;
            m_next = 0;
        }
            
    }
    
    QUrl url = QUrl(path);
    
    if(url.isValid())
    {
        QString localPath = url.toString(QUrl::PreferLocalFile);
            
        if (!isFile(localPath))
        {
            m_filename = path.section('/', -1);
            
            QString cachedFile = getCacheDirectory()+"/"+hash(path)+"_"+m_filename;
            
            if(isFile(cachedFile))
            {
                // File has been cached
                qDebug() << path << "is cached as" << cachedFile;
                
                if(successCallback.isCallable())
                {
                    args << QJSValue(cachedFile);
                    successCallback.call(args);
                }
                return;
            }
            
            m_successCallback = successCallback;
            m_errorCallback = errorCallback;
            m_filename = cachedFile;
            
            qDebug() << path << "doesn't exist locally, trying remote.";
            
            KIO::StoredTransferJob * job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo);
            connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
            
        }
        else
        {
            if(successCallback.isCallable())
            {
                args << QJSValue(path);
                successCallback.call(args);
            }
            return;
        }
    }
    else
    {
        errorMessage = path+" is not a valid URL";
        qCritical() << errorMessage;
        
        if(errorCallback.isCallable())
        {
            args << QJSValue(errorMessage);
            errorCallback.call(args);
        }
        return;
    }
    
}

void MediaFrame::slotItemChanged(const QString &path)
{
    emit itemChanged(path);
}

void MediaFrame::slotFinished(KJob *job)
{
    QString errorMessage = QString("");
    QJSValueList args;
    
    if (job->error())
    {
        errorMessage = "Error loading image: " + job->errorString();
        qCritical() << errorMessage;
        
        if(m_errorCallback.isCallable())
        {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    }
    else if (KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob *>(job))
    {
        QImage image;
        
        // TODO make proper caching calls
        QString path = m_filename;
        qDebug() << "Saving download to" << path;
        
        image.loadFromData(transferJob->data());
        image.save(path);
        
        qDebug() << "Saved to" << path;
        
        if(m_successCallback.isCallable())
        {
            args << QJSValue(path);
            m_successCallback.call(args);
        }
    }
    else
    {
        errorMessage = "Unknown error occured";
        
        qCritical() << errorMessage;
        
        if(m_errorCallback.isCallable())
        {
            args << QJSValue(errorMessage);
            m_errorCallback.call(args);
        }
    }
}
