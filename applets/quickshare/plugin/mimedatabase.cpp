/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mimedatabase.h"
#include <QDebug>
#include <QJsonObject>

static QJsonObject mimetypeToJsonObject(const QMimeType &type)
{
    if (!type.isValid()) {
        qWarning() << "trying to export an invalid type";
        return QJsonObject();
    }
    QJsonObject ret;
    ret[QStringLiteral("name")] = type.name();
    ret[QStringLiteral("iconName")] = type.iconName();
    ret[QStringLiteral("comment")] = type.comment();
    return ret;
}

MimeDatabase::MimeDatabase(QObject *parent)
    : QObject(parent)
{
}

QJsonObject MimeDatabase::mimeTypeForUrl(const QUrl &url) const
{
    return mimetypeToJsonObject(m_db.mimeTypeForUrl(url));
}

QJsonObject MimeDatabase::mimeTypeForName(const QString &name) const
{
    QMimeType type = m_db.mimeTypeForName(name);
    if (!type.isValid()) {
        qWarning() << "wrong mime name" << name;
        return QJsonObject();
    }
    return mimetypeToJsonObject(type);
}

#include "moc_mimedatabase.cpp"
