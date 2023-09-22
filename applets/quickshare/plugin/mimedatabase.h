/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MIMEDATABASE_H
#define MIMEDATABASE_H

#include <QMimeDatabase>
#include <QObject>

class MimeDatabase : public QObject
{
    Q_OBJECT
public:
    MimeDatabase(QObject *parent = nullptr);

    Q_SCRIPTABLE QJsonObject mimeTypeForUrl(const QUrl &url) const;
    Q_SCRIPTABLE QJsonObject mimeTypeForName(const QString &name) const;

private:
    QMimeDatabase m_db;
};

#endif // MIMEDATABASE_H
