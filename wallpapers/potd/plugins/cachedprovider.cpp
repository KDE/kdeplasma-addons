/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "cachedprovider.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThreadPool>
#include <QTimer>

#include <QDebug>
#include "debug.h"

LoadImageThread::LoadImageThread(const QString &filePath)
    : m_filePath(filePath)
{
}

void LoadImageThread::run()
{
    PotdProviderData data;
    data.wallpaperImage = QImage(m_filePath);

    const QString infoPath = m_filePath + QStringLiteral(".json");
    QFile infoFile(infoPath);

    if (infoFile.exists()) {
        if (infoFile.open(QIODevice::ReadOnly)) {
            QJsonParseError jsonParseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(infoFile.readAll(), &jsonParseError);
            infoFile.close();

            if (jsonParseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                const QJsonObject jsonObject = jsonDoc.object();
                data.wallpaperInfoUrl = QUrl(jsonObject.value(QStringLiteral("InfoUrl")).toString());
                data.wallpaperRemoteUrl = QUrl(jsonObject.value(QStringLiteral("RemoteUrl")).toString());
                data.wallpaperTitle = jsonObject.value(QStringLiteral("Title")).toString();
                data.wallpaperAuthor = jsonObject.value(QStringLiteral("Author")).toString();
            } else {
                qCWarning(WALLPAPERPOTD) << "Failed to read the wallpaper information!";
            }
        } else {
            qCWarning(WALLPAPERPOTD) << "Failed to open the wallpaper information file!";
        }
    }

    Q_EMIT done(data);
}

SaveImageThread::SaveImageThread(const QString &identifier, const PotdProviderData &data)
    : m_identifier(identifier)
    , m_data(data)
{
}

void SaveImageThread::run()
{
    m_data.wallpaperLocalUrl = CachedProvider::identifierToPath(m_identifier);
    m_data.wallpaperImage.save(m_data.wallpaperLocalUrl, "JPEG");

    const QString infoPath = m_data.wallpaperLocalUrl + ".json";
    QFile infoFile(infoPath);
    if (infoFile.open(QIODevice::WriteOnly)) {
        QJsonObject jsonObject;

        jsonObject.insert(QStringLiteral("InfoUrl"), m_data.wallpaperInfoUrl.url());
        jsonObject.insert(QStringLiteral("RemoteUrl"), m_data.wallpaperRemoteUrl.url());
        jsonObject.insert(QStringLiteral("Title"), m_data.wallpaperTitle);
        jsonObject.insert(QStringLiteral("Author"), m_data.wallpaperAuthor);

        infoFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
        infoFile.close();
    } else {
        qWarning(WALLPAPERPOTD) << "Failed to save the wallpaper information!";
    }

    Q_EMIT done(m_identifier, m_data);
}

QString CachedProvider::identifierToPath(const QString &identifier)
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/plasma_engine_potd/");
    QDir d;
    d.mkpath(dataDir);
    return dataDir + identifier;
}

CachedProvider::CachedProvider(const QString &identifier, QObject *parent)
    : PotdProvider(parent, KPluginMetaData(), QVariantList())
    , mIdentifier(identifier)
{
    LoadImageThread *thread = new LoadImageThread(identifierToPath(mIdentifier));
    connect(thread, &LoadImageThread::done, this, &CachedProvider::triggerFinished);
    QThreadPool::globalInstance()->start(thread);
}

CachedProvider::~CachedProvider()
{
}

QString CachedProvider::identifier() const
{
    return mIdentifier;
}

void CachedProvider::triggerFinished(const PotdProviderData &data)
{
    potdProviderData()->wallpaperImage = data.wallpaperImage;
    potdProviderData()->wallpaperLocalUrl = data.wallpaperLocalUrl;
    potdProviderData()->wallpaperInfoUrl = data.wallpaperInfoUrl;
    potdProviderData()->wallpaperRemoteUrl = data.wallpaperRemoteUrl;
    potdProviderData()->wallpaperTitle = data.wallpaperTitle;
    potdProviderData()->wallpaperAuthor = data.wallpaperAuthor;

    Q_EMIT finished(this);
}

bool CachedProvider::isCached(const QString &identifier, bool ignoreAge)
{
    const QString path = identifierToPath(identifier);
    if (!QFile::exists(path)) {
        return false;
    }

    QRegularExpression re(QLatin1String(":\\d{4}-\\d{2}-\\d{2}"));

    if (!ignoreAge && !re.match(identifier).hasMatch()) {
        // no date in the identifier, so it's a daily; check to see ifthe modification time is today
        QFileInfo info(path);
        if (info.lastModified().daysTo(QDateTime::currentDateTime()) >= 1) {
            return false;
        }
    }

    return true;
}
