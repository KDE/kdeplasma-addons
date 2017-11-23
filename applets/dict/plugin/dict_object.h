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
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>
#include <QObject>
#include <vector>
class QQuickWebEngineProfile;

class DictObject : public QObject, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineProfile* webProfile READ webProfile CONSTANT)

public:
    explicit DictObject(QObject *parent = nullptr);

    QQuickWebEngineProfile* webProfile() const;

public Q_SLOTS:
    void lookup(const QString &word);

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void searchInProgress();
    void definitionFound(const QString &html);

private:
    QString m_source;
    QString m_dataEngine;
    struct AvailableDict {
        QString id;
        QString description;
    };
    std::vector<AvailableDict> m_availableDicts;
    QString m_selectedDict;

    QQuickWebEngineProfile* m_webProfile;
};

#endif
