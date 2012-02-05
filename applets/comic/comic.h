/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
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

#include <QtCore/QDate>

#include <KUrl>
#include <Plasma/DataEngine>
#include <Plasma/PopupApplet>

class ArrowWidget;
class ButtonBar;
class CheckNewStrips;
class ComicLabel;
class ComicModel;
class ComicTabBar;
class ConfigWidget;
class FullViewWidget;
class ImageWidget;
class KAction;
class KJob;
class QAction;
class QGraphicsLayout;
class QSortFilterProxyModel;
class QTimer;
class SavingDir;

class ComicApplet : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet();

        void init();
        virtual QList<QAction*> contextualActions();

        QGraphicsWidget *graphicsWidget();

    public Q_SLOTS:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
        void createConfigurationInterface( KConfigDialog *parent );

    private Q_SLOTS:
        void slotTabChanged( int newIndex );
        void slotChosenDay( const QDate &date );
        void slotNextDay();
        void slotPreviousDay();
        void slotFirstDay();
        void slotCurrentDay();
        void slotFoundLastStrip( int index, const QString &identifier, const QString &suffix );
        void slotGoJump();
        void slotNextNewStrip();
        void slotReload();
        void slotSaveComicAs();
        void slotScaleToContent();
        void slotShop();
        void slotStorePosition();
        void slotSizeChanged();
        void slotShowMaxSize();
        void applyConfig();
        void checkDayChanged();
        void buttonBar();
        void fullView();
        void updateSize();
        void createComicBook();
        void slotArchive( int archiveType, const KUrl &dest, const QString &fromIdentifier, const QString &toIdentifier );
        void slotArchiveFinished( KJob *job );

    public slots:
        void configChanged();

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
        bool eventFilter( QObject *receiver, QEvent *event );

    private:
        void changeComic( bool differentComic );
        void updateComic( const QString &identifierSuffix = QString() );
        void updateUsedComics();
        void updateButtons();
        void updateContextMenu();
        void updateView();
        void saveConfig();
        bool isInPanel() const;
        void setTabBarVisible( bool isVisible );//HACK what was in 4.4 does not seem to work anymore, so this was added

    private:
        static const int CACHE_LIMIT;
        ComicModel *mModel;
        QSortFilterProxyModel *mProxy;

        QDate mCurrentDay;

        QString mIdentifierError;
        QString mOldSource;
        ConfigWidget *mConfigWidget;
        bool mDifferentComic;
        bool mShowPreviousButton;
        bool mShowNextButton;
        bool mShowComicUrl;
        bool mShowComicAuthor;
        bool mShowComicTitle;
        bool mShowComicIdentifier;
        bool mShowErrorPicture;
        bool mArrowsOnHover;
        bool mMiddleClick;
        int mCheckNewComicStripsIntervall;
        CheckNewStrips *mCheckNewStrips;
        QTimer *mDateChangedTimer;
        QList<QAction*> mActions;
        QGraphicsWidget *mMainWidget;
        QGraphicsLinearLayout *mCentralLayout;
        QGraphicsLinearLayout *mBottomLayout;
        FullViewWidget *mFullViewWidget;
        QAction *mActionGoFirst;
        QAction *mActionGoLast;
        QAction *mActionGoJump;
        QAction *mActionScaleContent;
        QAction *mActionShop;
        QAction *mActionStorePosition;
        KAction *mActionNextNewStripTab;
        QSizeF mMaxSize;
        QSizeF mLastSize;
        QSizeF mIdealSize;
        Plasma::DataEngine *mEngine;
        ComicLabel *mLabelId;
        ComicLabel *mLabelTop;
        ComicLabel *mLabelUrl;

        ImageWidget *mImageWidget;
        ArrowWidget *mLeftArrow;
        ArrowWidget *mRightArrow;

        //Tabs
        bool mTabAdded;
        ComicTabBar *mTabBar;
        QStringList mTabIdentifier;

        enum TabView {
            ShowText = 0x1,
            ShowIcon = 0x2
        };
        int mTabView;

        ComicData mCurrent;
        ButtonBar *mButtonBar;
        SavingDir *mSavingDir;
};

K_EXPORT_PLASMA_APPLET( comic, ComicApplet )

#endif
