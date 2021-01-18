/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>
#include <QObject>
class QQuickWebEngineProfile;

class DictObject : public QObject, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineProfile *webProfile READ webProfile CONSTANT)
    Q_PROPERTY(QString selectedDictionary READ selectedDictionary WRITE setSelectedDictionary)

public:
    explicit DictObject(QObject *parent = nullptr);

    QQuickWebEngineProfile *webProfile() const;

    QString selectedDictionary() const;
    void setSelectedDictionary(const QString &dict);

public Q_SLOTS:
    void lookup(const QString &word);

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void searchInProgress();
    void definitionFound(const QString &html);

private:
    QString m_source;
    QString m_dataEngineName;
    QString m_selectedDict;

    Plasma::DataEngine *m_dataEngine;
    QQuickWebEngineProfile *m_webProfile;
};

#endif
