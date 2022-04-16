/*
    SPDX-FileCopyrightText: 2007 Jeff Cooper <weirdsox11@gmail.com>
    SPDX-FileCopyrightText: 2007 Thomas Georgiou <TAGeorgiou@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dictengine.h"
#include <iostream>

#include <KLocalizedString>
#include <QDebug>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QUrl>

DictEngine::DictEngine(QObject *parent)
    : QObject(parent)
    , m_dictName(QStringLiteral("wn")) // In case we need to switch it later
    , m_serverName(QStringLiteral("dict.org")) // Default, good dictionary
{
}

DictEngine::~DictEngine()
{
}

void DictEngine::setDict(const QString &dict)
{
    m_dictName = dict;
}

void DictEngine::setServer(const QString &server)
{
    m_serverName = server;
}

static QString wnToHtml(const QString &word, QByteArray &text)
{
    QList<QByteArray> splitText = text.split('\n');
    QString def;
    def += QLatin1String("<dl>\n");
    static QRegularExpression linkRx(QStringLiteral("{(.*?)}"));

    bool isFirst = true;
    while (!splitText.empty()) {
        // 150 n definitions retrieved - definitions follow
        // 151 word database name - text follows
        // 250 ok (optional timing information here)
        // 552 No match
        QString currentLine = splitText.takeFirst();
        if (currentLine.startsWith(QLatin1String("151"))) {
            isFirst = true;
            continue;
        }

        if (currentLine.startsWith('.')) {
            def += QLatin1String("</dd>");
            continue;
        }

        if (currentLine.startsWith("552") || currentLine.startsWith("501")) {
            return i18n("No match found for %1", word);
        }

        if (!(currentLine.startsWith(QLatin1String("150")) || currentLine.startsWith(QLatin1String("151")) || currentLine.startsWith(QLatin1String("250")))) {
            // Handle links
            int offset = 0;
            QRegularExpressionMatchIterator it = linkRx.globalMatch(currentLine);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QUrl url;
                url.setScheme("dict");
                url.setPath(match.captured(1));
                const QString linkText = QStringLiteral("<a href=\"%1\">%2</a>").arg(url.toString(), match.captured(1));
                currentLine.replace(match.capturedStart(0) + offset, match.capturedLength(0), linkText);
                offset += linkText.length() - match.capturedLength(0);
            }

            if (isFirst) {
                def += "<dt><b>" + currentLine + "</b></dt>\n<dd>";
                isFirst = false;
                continue;
            } else {
                static QRegularExpression newLineRx(QStringLiteral("([1-9]{1,2}:)"));
                if (currentLine.contains(newLineRx)) {
                    def += QLatin1String("\n<br>\n");
                }
                static QRegularExpression makeMeBoldRx(QStringLiteral("^([\\s\\S]*[1-9]{1,2}:)"));
                currentLine.replace(makeMeBoldRx, QLatin1String("<b>\\1</b>"));
                def += currentLine;
                continue;
            }
        }
    }

    def += QLatin1String("</dl>");
    return def;
}

void DictEngine::getDefinition()
{
    m_tcpSocket->readAll();
    QByteArray ret;

    const QByteArray command = QByteArray("DEFINE ") + m_dictName.toLatin1() + " \"" + m_currentWord.toUtf8() + "\"\n";
    // qDebug() << command;
    m_tcpSocket->write(command);
    m_tcpSocket->flush();

    while (!ret.contains("250") && !ret.contains("552") && !ret.contains("550") && !ret.contains("501") && !ret.contains("503")) {
        m_tcpSocket->waitForReadyRead();
        ret += m_tcpSocket->readAll();
    }

    m_tcpSocket->disconnectFromHost();
    const QString html = wnToHtml(m_currentWord, ret);
    Q_EMIT definitionRecieved(html);
}

void DictEngine::getDicts()
{
    m_tcpSocket->readAll();
    QByteArray ret;

    m_tcpSocket->write(QByteArray("SHOW DB\n"));
    m_tcpSocket->flush();

    m_tcpSocket->waitForReadyRead();
    while (!ret.contains("250") || !ret.contains("420") || !ret.contains("421")) {
        m_tcpSocket->waitForReadyRead();
        ret += m_tcpSocket->readAll();
    }

    QMap<QString, QString> availableDicts;
    const QList<QByteArray> retLines = ret.split('\n');
    for (const QByteArray &curr : retLines) {
        if (curr.endsWith("420") || curr.startsWith("421")) {
            // TODO: what happens if the server is down
        }
        if (curr.startsWith("554")) {
            // TODO: What happens if no DB available?
            // TODO: Eventually there will be functionality to change the server...
            break;
        }

        // ignore status code and empty lines
        if (curr.startsWith("250") || curr.startsWith("110") || curr.isEmpty()) {
            continue;
        }

        if (!curr.startsWith('-') && !curr.startsWith('.')) {
            const QString line = QString::fromUtf8(curr).trimmed();
            const QString id = line.section(' ', 0, 0);
            QString description = line.section(' ', 1);
            if (description.startsWith('"') && description.endsWith('"')) {
                description.remove(0, 1);
                description.chop(1);
            }
            availableDicts.insert(id, description);
        }
    }

    m_tcpSocket->disconnectFromHost();
    m_availableDictsCache.insert(m_serverName, availableDicts);
    Q_EMIT dictsRecieved(availableDicts);
}

void DictEngine::requestDicts()
{
    if (m_availableDictsCache.contains(m_serverName)) {
        Q_EMIT dictsRecieved(m_availableDictsCache.value(m_serverName));
        return;
    }
    if (m_tcpSocket) {
        m_tcpSocket->abort(); // stop if lookup is in progress and new query is requested
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
    }

    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DictEngine::socketClosed);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DictEngine::getDicts);
    m_tcpSocket->connectToHost(m_serverName, 2628);
}
void DictEngine::requestDefinition(const QString &query)
{
    if (m_tcpSocket) {
        m_tcpSocket->abort(); // stop if lookup is in progress and new query is requested
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
        Q_EMIT definitionRecieved(QString());
    }

    QStringList queryParts = query.split(':', Qt::SkipEmptyParts);
    if (queryParts.isEmpty()) {
        return;
    }

    m_currentWord = queryParts.last();
    m_currentQuery = query;

    // asked for a dictionary?
    if (queryParts.count() > 1) {
        setDict(queryParts[queryParts.count() - 2]);
        // default to wordnet
    } else {
        setDict(QStringLiteral("wn"));
    }

    // asked for a server?
    if (queryParts.count() > 2) {
        setServer(queryParts[queryParts.count() - 3]);
        // default to wordnet
    } else {
        setServer(QStringLiteral("dict.org"));
    }
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DictEngine::socketClosed);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DictEngine::getDefinition);
    m_tcpSocket->connectToHost(m_serverName, 2628);
}

void DictEngine::socketClosed()
{
    if (m_tcpSocket) {
        m_tcpSocket->deleteLater();
    }
    m_tcpSocket = nullptr;
}

#include "dictengine.moc"
