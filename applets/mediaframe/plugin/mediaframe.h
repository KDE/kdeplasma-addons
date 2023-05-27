/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MEDIAFRAME_H
#define MEDIAFRAME_H

#include <QFileSystemWatcher>
#include <QHash>
#include <QJSValue>
#include <QObject>
#include <QString>
#include <QStringList>

#include <KIO/Job>

class MediaFrame : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int historyLength READ historyLength NOTIFY historyLengthChanged)
    Q_PROPERTY(int futureLength READ futureLength NOTIFY futureLengthChanged)
    Q_PROPERTY(bool random READ random WRITE setRandom NOTIFY randomChanged)
    Q_PROPERTY(QStringList paths READ paths WRITE setPaths NOTIFY pathsChanged)

public:
    explicit MediaFrame(QObject *parent = nullptr);
    ~MediaFrame() override;

    int count() const;

    int historyLength() const;
    int futureLength() const;

    bool random() const;
    void setRandom(bool random);

    QStringList paths() const;
    void setPaths(const QStringList &paths);

    Q_INVOKABLE bool isDir(const QString &path);
    Q_INVOKABLE bool isDirEmpty(const QString &path);
    Q_INVOKABLE bool isFile(const QString &path);

    Q_INVOKABLE QStringList add(const QVariant &data) const;
    Q_INVOKABLE void clear();

    Q_INVOKABLE void watch(const QString &path);

    bool isAdded(const QString &path) const;

    Q_INVOKABLE void get(QJSValue callback);
    Q_INVOKABLE void get(QJSValue callback, QJSValue error_callback);

    Q_INVOKABLE void pushHistory(const QString &string);
    Q_INVOKABLE QString popHistory();

    Q_INVOKABLE void pushFuture(const QString &string);
    Q_INVOKABLE QString popFuture();

Q_SIGNALS:
    void countChanged();
    void historyLengthChanged();
    void futureLengthChanged();
    void randomChanged();
    void pathsChanged();
    void itemChanged(const QString &path);

private Q_SLOTS:
    void slotItemChanged(const QString &path);
    void slotFinished(KJob *job);

private:
    int random(int min, int max);
    QString getCacheDirectory();
    QString hash(const QString &str);

    QStringList m_filters;
    QStringList m_paths;
    QHash<QString, QStringList> m_pathMap;
    QStringList m_allFiles;
    QString m_watchFile;
    QFileSystemWatcher m_watcher;

    QStringList m_history;
    QStringList m_future;

    QJSValue m_successCallback;
    QJSValue m_errorCallback;
    QString m_filename;

    bool m_random = false;
    int m_next = 0;
};

#endif
