/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
 *   Copyright (C) 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>      *
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef COMIC_H
#define COMIC_H

#include "comicdata.h"

#include <QDate>
#include <QUrl>

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>
#include <Plasma/Applet>

#include "activecomicmodel.h"

class CheckNewStrips;
class ComicModel;
class ConfigWidget;
class QAction;
class KJob;
class QAction;
class QSortFilterProxyModel;
class QTimer;
class SavingDir;

namespace KNS3 {
    class DownloadDialog;
}

class ComicApplet : public Plasma::Applet, public Plasma::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(QObject *comicsModel READ comicsModel NOTIFY comicModelChanged)
    Q_PROPERTY(QObject *availableComicsModel READ availableComicsModel CONSTANT)
    Q_PROPERTY(bool showComicUrl READ showComicUrl WRITE setShowComicUrl NOTIFY showComicUrlChanged)
    Q_PROPERTY(bool showComicAuthor READ showComicAuthor WRITE setShowComicAuthor NOTIFY showComicAuthorChanged)
    Q_PROPERTY(bool showComicTitle READ showComicTitle WRITE setShowComicTitle NOTIFY showComicTitleChanged)
    Q_PROPERTY(bool showComicIdentifier READ showComicIdentifier WRITE setShowComicIdentifier NOTIFY showComicIdentifierChanged)
    Q_PROPERTY(bool showErrorPicture READ showErrorPicture WRITE setShowErrorPicture NOTIFY showErrorPictureChanged)
    Q_PROPERTY(bool arrowsOnHover READ arrowsOnHover WRITE setArrowsOnHover NOTIFY arrowsOnHoverChanged)
    Q_PROPERTY(bool middleClick READ middleClick WRITE setMiddleClick NOTIFY middleClickChanged)
    Q_PROPERTY(QVariantMap comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)
    Q_PROPERTY(QStringList tabIdentifiers READ tabIdentifiers WRITE setTabIdentifiers NOTIFY tabIdentifiersChanged)
    Q_PROPERTY(int checkNewComicStripsInterval READ checkNewComicStripsInterval WRITE setCheckNewComicStripsInterval NOTIFY checkNewComicStripsIntervalChanged)
    Q_PROPERTY(int providerUpdateInterval READ providerUpdateInterval WRITE setProviderUpdateInterval NOTIFY providerUpdateIntervalChanged)
    Q_PROPERTY(int maxComicLimit READ maxComicLimit WRITE setMaxComicLimit NOTIFY maxComicLimitChanged)

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet() override;

        void init() override;
        QList<QAction*> contextualActions() override;

        //For QML
        QObject *comicsModel() const;
        QObject *availableComicsModel() const;
        QVariantMap comicData() const;

        QStringList tabIdentifiers() const;
        void setTabIdentifiers(const QStringList &tabs);

        bool showComicUrl() const;
        void setShowComicUrl(bool show);

        bool showComicAuthor() const;
        void setShowComicAuthor(bool show);

        bool showComicTitle() const;
        void setShowComicTitle(bool show);

        bool showComicIdentifier() const;
        void setShowComicIdentifier(bool show);

        bool showErrorPicture() const;
        void setShowErrorPicture(bool show);

        bool arrowsOnHover() const;
        void setArrowsOnHover(bool show);

        bool middleClick() const;
        void setMiddleClick(bool show);

        bool showActualSize() const;
        void setShowActualSize(bool show);

        int checkNewComicStripsInterval() const;
        void setCheckNewComicStripsInterval(int interval);

        int providerUpdateInterval() const;
        void setProviderUpdateInterval(int interval);

        void setMaxComicLimit(int limit);
        int maxComicLimit() const;
        //End for QML

Q_SIGNALS:
    void comicModelChanged();
    void showComicUrlChanged();
    void showComicAuthorChanged();
    void showComicTitleChanged();
    void showComicIdentifierChanged();
    void showErrorPictureChanged();
    void arrowsOnHoverChanged();
    void middleClickChanged();
    void comicDataChanged();
    void tabHighlightRequest(const QString &id, bool highlight);
    void showNextNewStrip();
    void showActualSizeChanged();
    void tabIdentifiersChanged();
    void checkNewComicStripsIntervalChanged();
    void providerUpdateIntervalChanged();
    void maxComicLimitChanged();

    public Q_SLOTS:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );

    private Q_SLOTS:
        void slotTabChanged( const QString &newIdentifier );
        void slotNextDay();
        void slotPreviousDay();
        void slotFirstDay();
        void slotCurrentDay();
        void slotFoundLastStrip( int index, const QString &identifier, const QString &suffix );
        void slotGoJump();
        void slotSaveComicAs();
        void slotScaleToContent();
        void slotShop();
        void slotStorePosition();
        void checkDayChanged();
        void createComicBook();
        void slotArchive( int archiveType, const QUrl &dest, const QString &fromIdentifier, const QString &toIdentifier );
        void slotArchiveFinished( KJob *job );

    public Q_SLOTS:
        void configChanged() override;
        void saveConfig();
        Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());
        Q_INVOKABLE void goJump() { slotGoJump();}
        Q_INVOKABLE void shop() { slotShop();}
        Q_INVOKABLE void tabChanged(const QString &newIdentifier) { slotTabChanged(newIdentifier);}
        Q_INVOKABLE void getNewComics();
        Q_INVOKABLE void positionFullView(QWindow *window);

    private:
        void changeComic( bool differentComic );
        void updateUsedComics();
        void updateContextMenu();
        void updateView();
        void refreshComicData();
        void setTabHighlighted(const QString &id, bool highlight);
        bool isTabHighlighted(const QString &id) const;

    private:
        static const int CACHE_LIMIT;
        ComicModel *mModel;
        QSortFilterProxyModel *mProxy;
        ActiveComicModel *mActiveComicModel;
        QVariantMap mComicData;

        QDate mCurrentDay;

        QString mIdentifierError;
        QString mOldSource;
        ConfigWidget *mConfigWidget;
        bool mDifferentComic;
        bool mShowComicUrl;
        bool mShowComicAuthor;
        bool mShowComicTitle;
        bool mShowComicIdentifier;
        bool mShowErrorPicture;
        bool mArrowsOnHover;
        bool mMiddleClick;
        int mCheckNewComicStripsInterval;
        int mMaxComicLimit;
        CheckNewStrips *mCheckNewStrips;
        QTimer *mDateChangedTimer;
        QList<QAction*> mActions;
        QAction *mActionGoFirst;
        QAction *mActionGoLast;
        QAction *mActionGoJump;
        QAction *mActionScaleContent;
        QAction *mActionShop;
        QAction *mActionStorePosition;
        QAction *mActionNextNewStripTab;
        QAction *mActionSaveComicAs;
        QAction *mActionCreateComicBook;
        QSizeF mMaxSize;
        QSizeF mLastSize;
        QSizeF mIdealSize;
        Plasma::DataEngine *mEngine;
        QPointer<KNS3::DownloadDialog> m_newStuffDialog;

        //Tabs
        bool mTabAdded;
        QStringList mTabIdentifier;

        ComicData mCurrent;
        SavingDir *mSavingDir;
};

#endif
