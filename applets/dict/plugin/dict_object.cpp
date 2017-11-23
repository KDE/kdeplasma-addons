/*
 * Copyright (C) 2017 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dict_object.h"
#include <QDebug>
#include <KLocalizedString>
#include <QQuickWebEngineProfile>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>


// The KDE4 applet could use "qstardict" if available, but I have no idea where this came from.
static const char defaultDataEngine[] = "dict";

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    DictSchemeHandler(QObject *parent)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    void requestStarted(QWebEngineUrlRequestJob *job) {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
        const QString word = job->requestUrl().path();
        emit wordClicked(word);
   }

signals:
    void wordClicked(const QString &word);
};

DictObject::DictObject(QObject *parent)
    : QObject(parent),
    m_dataEngine(QString::fromLatin1(defaultDataEngine))
{
    dataEngine(m_dataEngine)->connectSource(QLatin1String("list-dictionaries"), this);

    m_selectedDict = QStringLiteral("wn"); // for testing, use e.g. fd-fra-eng
    m_webProfile = new QQuickWebEngineProfile(this);
    auto *schemeHandler = new DictSchemeHandler(this);
    connect(schemeHandler, &DictSchemeHandler::wordClicked,
            this, &DictObject::lookup);
    m_webProfile->installUrlSchemeHandler("dict", schemeHandler);
}

void DictObject::lookup(const QString &word)
{
    const QString newSource = m_selectedDict + ':' + word;

    if (!m_source.isEmpty()) {
        dataEngine(m_dataEngine)->disconnectSource(m_source, this);
    }

    if (!newSource.isEmpty()) {
        // Look up new definition
        emit searchInProgress();
        m_source = newSource;
        dataEngine(m_dataEngine)->connectSource(m_source, this);
    }
}

void DictObject::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    if (sourceName == QLatin1String("list-dictionaries")) {
        m_availableDicts = {};
        m_availableDicts.resize(data.count());
        int i = 0;
        for (auto it = data.begin(), end = data.end(); it != end; ++it, ++i) {
            m_availableDicts[i] = AvailableDict{it.key(), it.value().toString()};
        }
    } else {
        const QString html = data.value(QStringLiteral("text")).toString();
        if (!html.isEmpty()) {
            emit definitionFound(html);
        }
    }
}

QQuickWebEngineProfile* DictObject::webProfile() const
{
    return m_webProfile;
}

#include "dict_object.moc"
