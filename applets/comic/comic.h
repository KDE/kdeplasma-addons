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

#include <QDate>
#include <QUrl>

#include <Plasma/Applet>

#include "comicdata.h"
#include "engine/comic.h"

class CheckNewStrips;
class ComicModel;
class QAction;
class KJob;
class QAction;
class QSortFilterProxyModel;
class QTimer;
class SavingDir;

class ComicApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject *availableComicsModel READ availableComicsModel CONSTANT)
    Q_PROPERTY(QVariantMap comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)

public:
    ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    void init() override;

    // For QML
    QObject *availableComicsModel() const;
    QVariantMap comicData() const;

    bool showActualSize() const;
    void setShowActualSize(bool show);

    Q_INVOKABLE bool urlExists(const QUrl &url);
    Q_INVOKABLE bool saveImage(const QUrl &fileUrl);

    // End for QML

Q_SIGNALS:
    void comicDataChanged();
    void showNextNewStrip();
    void showActualSizeChanged();

private Q_SLOTS:
    void slotTabChanged(const QString &newIdentifier);
    void slotFoundLastStrip(int index, const QString &identifier, const QString &suffix);
    void slotGoJump();
    void slotShowActualSize(bool show);
    void slotShop();
    void slotWebsite();
    void slotStorePosition(bool store);
    void checkDayChanged();

public Q_SLOTS:
    void configChanged() override;
    void saveConfig();
    Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());

    Q_INVOKABLE void goJump()
    {
        slotGoJump();
    }

    Q_INVOKABLE void shop()
    {
        slotShop();
    }

    Q_INVOKABLE void website()
    {
        slotWebsite();
    }

    Q_INVOKABLE void storePosition(bool store)
    {
        slotStorePosition(store);
    }

    Q_INVOKABLE void tabChanged(const QString &newIdentifier)
    {
        slotTabChanged(newIdentifier);
    }

    Q_INVOKABLE void loadProviders();
    Q_INVOKABLE void positionFullView(QWindow *window);

private:
    void updateUsedComics();
    void refreshComicData();
    void setTabHighlighted(const QString &id, bool highlight);
    void dataUpdated(const ComicMetaData &data);

private:
    ComicModel *mModel = nullptr;
    QString mPreviousFailedIdentifier;
    QVariantMap mComicData;

    QDate mCurrentDay;

    QString mOldSource;
    CheckNewStrips *mCheckNewStrips;
    QTimer *mDateChangedTimer;
    ComicEngine *const mEngine;

    ComicData mCurrent;
};

#endif
