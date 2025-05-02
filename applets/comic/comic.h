/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_H
#define COMIC_H

#include <QUrl>

#include <Plasma/Applet>

#include "comicdata.h"
#include "engine/comic.h"

class CheckNewStrips;
class ComicModel;

class ComicApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject *availableComicsModel READ availableComicsModel CONSTANT)
    Q_PROPERTY(ComicData comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)

public:
    ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    void init() override;

    // For QML
    QObject *availableComicsModel() const;
    ComicData comicData() const;

    bool showActualSize() const;
    void setShowActualSize(bool show);

    Q_INVOKABLE bool urlExists(const QUrl &url);
    Q_INVOKABLE void storePosition(bool store);
    Q_INVOKABLE void tabChanged(const QString &newIdentifier);
    Q_INVOKABLE void loadProviders();
    Q_INVOKABLE void positionFullView(QWindow *window);

    // End for QML

Q_SIGNALS:
    void comicDataChanged();
    void showActualSizeChanged();

private Q_SLOTS:
    void slotFoundLastStrip(int index, const QString &identifier, const QString &suffix);

public Q_SLOTS:
    void configChanged() override;
    Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());

private:
    void updateUsedComics();
    void setTabHighlighted(const QString &id, bool highlight);
    void dataUpdated(const ComicMetaData &data);

private:
    ComicEngine *const mEngine;
    ComicModel *mModel = nullptr;

    CheckNewStrips *mCheckNewStrips;

    ComicData mCurrent;
};

#endif
