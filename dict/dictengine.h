/*
    SPDX-FileCopyrightText: 2007 Jeff Cooper <weirdsox11@gmail.com>
    SPDX-FileCopyrightText: 2007 Thomas Georgiou <TAGeorgiou@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QMap>
#include <QObject>
#include <QVariantMap>

class QTcpSocket;

/**
 * This class evaluates the basic expressions given in the interface.
 */

class DictEngine : public QObject
{
    Q_OBJECT

public:
    DictEngine(QObject *parent = nullptr);
    ~DictEngine() override;

Q_SIGNALS:
    void dictsRecieved(const QMap<QString, QString> &dicts);
    void definitionRecieved(const QString &html);
public Q_SLOTS:
    void requestDicts();
    void requestDefinition(const QString &query);

private:
    void getDefinition();
    void socketClosed();
    void getDicts();
    void setDict(const QString &dict);
    void setServer(const QString &server);

    QHash<QString, QString> m_dictNameToDictCode;
    QTcpSocket *m_tcpSocket = nullptr;
    QString m_currentWord;
    QString m_currentQuery;
    QString m_dictName;
    QString m_serverName;
    QMap<QString, QMap<QString, QString>> m_availableDictsCache;

    // https://datatracker.ietf.org/doc/html/rfc2229
    std::array<QByteArray, 5> m_definitionResponses;
};
