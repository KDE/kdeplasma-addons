/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
    void requestStarted(QWebEngineUrlRequestJob *job) override {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
        const QString word = job->requestUrl().path();
        emit wordClicked(word);
   }

Q_SIGNALS:
    void wordClicked(const QString &word);
};

DictObject::DictObject(QObject *parent)
    : QObject(parent),
    m_dataEngineName(QString::fromLatin1(defaultDataEngine))
{
    m_selectedDict = QStringLiteral("wn");
    m_webProfile = new QQuickWebEngineProfile(this);
    auto *schemeHandler = new DictSchemeHandler(this);
    connect(schemeHandler, &DictSchemeHandler::wordClicked,
            this, &DictObject::lookup);
    m_webProfile->installUrlSchemeHandler("dict", schemeHandler);
    m_dataEngine = dataEngine(m_dataEngineName); // Load it upfront so the config dialog can reuse this one
}

void DictObject::lookup(const QString &word)
{
    const QString newSource = m_selectedDict + QLatin1Char(':') + word;

    if (!m_source.isEmpty()) {
        m_dataEngine->disconnectSource(m_source, this);
    }

    if (!newSource.isEmpty()) {
        // Look up new definition
        emit searchInProgress();
        m_source = newSource;
        m_dataEngine->connectSource(m_source, this);
    }
}

void DictObject::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName); // always == m_source
    const QString html = data.value(QStringLiteral("text")).toString();
    if (!html.isEmpty()) {
        emit definitionFound(html);
    }
}

QString DictObject::selectedDictionary() const
{
    return m_selectedDict;
}

void DictObject::setSelectedDictionary(const QString& dict)
{
    m_selectedDict = dict;
}


QQuickWebEngineProfile* DictObject::webProfile() const
{
    return m_webProfile;
}

#include "dict_object.moc"
