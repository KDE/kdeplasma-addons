/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMICMODEL_H
#define COMICMODEL_H

#include <QAbstractTableModel>

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>

class ComicModel : public QAbstractTableModel, public Plasma::DataEngineConsumer
{
    Q_OBJECT

public:
    ComicModel(Plasma::DataEngine *engine, const QString &source, const QStringList &usedComics, QObject *parent = nullptr);

    void setComics(const Plasma::DataEngine::Data &comics, const QStringList &usedComics);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    int columnCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::CheckStateRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

private:
    Plasma::DataEngine::Data mComics;
    QStringList mUsedComics;
};

#endif
