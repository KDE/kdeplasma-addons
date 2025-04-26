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

#include "activecomicmodel.h"
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
    Q_PROPERTY(QObject *comicsModel READ comicsModel NOTIFY comicModelChanged)
    Q_PROPERTY(QObject *availableComicsModel READ availableComicsModel CONSTANT)
    Q_PROPERTY(QVariantMap comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)

public:
    ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ComicApplet() override;

    void init() override;

    // For QML
    QObject *comicsModel() const;
    QObject *availableComicsModel() const;
    QVariantMap comicData() const;

    bool showActualSize() const;
    void setShowActualSize(bool show);

    // End for QML

Q_SIGNALS:
    void comicModelChanged();
    void comicDataChanged();
    void tabHighlightRequest(const QString &id, bool highlight);
    void showNextNewStrip();
    void showActualSizeChanged();

private Q_SLOTS:
    void slotTabChanged(const QString &newIdentifier);
    void slotFoundLastStrip(int index, const QString &identifier, const QString &suffix);
    void slotGoJump();
    void slotSaveComicAs();
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

    Q_INVOKABLE void saveComicAs()
    {
        slotSaveComicAs();
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
    bool isTabHighlighted(const QString &id) const;
    void dataUpdated(const ComicMetaData &data);

private:
    ComicModel *mModel = nullptr;
    QString mPreviousFailedIdentifier;
    ActiveComicModel *mActiveComicModel;
    QVariantMap mComicData;

    QDate mCurrentDay;

    QString mOldSource;
    CheckNewStrips *mCheckNewStrips;
    QTimer *mDateChangedTimer;
    ComicEngine *const mEngine;

    ComicData mCurrent;
    SavingDir *mSavingDir;
};

#endif
