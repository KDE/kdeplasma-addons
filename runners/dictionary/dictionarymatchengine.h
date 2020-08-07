/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYMATCHENGINE_H
#define DICTIONARYMATCHENGINE_H

#include <Plasma/DataEngine>
#include <QHash>
#include <QReadWriteLock>
#include <QMutex>
#include <QMultiMap>
#include <QWaitCondition>

namespace Plasma
{
}
class DictionaryMatchEngine : public QObject
{
    Q_OBJECT

public:
    explicit DictionaryMatchEngine(Plasma::DataEngine *dictionaryEngine, QObject *parent = nullptr);
    QString lookupWord(const QString &word);

private:
    struct ThreadData {
        QWaitCondition waitCondition;
        QMutex mutex;
        QString definition;
    };
    QMultiMap <QString, ThreadData*> m_lockers;
    QReadWriteLock m_wordLock;
    Plasma::DataEngine *m_dictionaryEngine;

private Q_SLOTS:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
    void sourceAdded(const QString &source);
    void sourceRemoved(const QString &source);

};

#endif
