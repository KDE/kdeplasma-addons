/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICTIONARIES_MODEL_H
#define DICTIONARIES_MODEL_H

#include "../../dict/dictengine.h"
#include <QAbstractListModel>
#include <QAbstractSocket>
#include <vector>

class DictionariesModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @return @c true if the engine is downloading dict list from
     * the Internet, @c false otherwise.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

    /**
     * @return the type of the last socket error
     */
    Q_PROPERTY(QAbstractSocket::SocketError errorCode READ errorCode NOTIFY errorCodeChanged)

    /**
     * @return a human-readable description of the last socket error
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit DictionariesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loading() const;

    QAbstractSocket::SocketError errorCode() const;
    QString errorString() const;

Q_SIGNALS:
    void loadingChanged();
    void errorCodeChanged();
    void errorStringChanged();

private Q_SLOTS:
    void slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString);
    void slotDictLoadingChanged(bool loading);

private:
    void setAvailableDicts(const QVariantMap &data);

    struct AvailableDict {
        QString id;
        QString description;
    };
    std::vector<AvailableDict> m_availableDicts;

    bool m_loading = false;
    QAbstractSocket::SocketError m_errorCode = QAbstractSocket::UnknownSocketError;
    QString m_errorString;
};

#endif
