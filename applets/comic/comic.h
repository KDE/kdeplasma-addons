/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
 *   Copyright (C) 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>      *
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

#include "activecomicmodel.h"

namespace Plasma {
    class DeclarativeWidget;
}

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
    Q_PROPERTY(QObject * comicsModel READ comicsModel)
    Q_PROPERTY(bool showComicUrl READ showComicUrl WRITE setShowComicUrl NOTIFY showComicUrlChanged)
    Q_PROPERTY(bool showComicAuthor READ showComicAuthor WRITE setShowComicAuthor NOTIFY showComicAuthorChanged)
    Q_PROPERTY(bool showComicTitle READ showComicTitle WRITE setShowComicTitle NOTIFY showComicTitleChanged)
    Q_PROPERTY(bool showComicIdentifier READ showComicIdentifier WRITE setShowComicIdentifier NOTIFY showComicIdentifierChanged)
    Q_PROPERTY(bool showErrorPicture READ showErrorPicture WRITE setShowErrorPicture NOTIFY showErrorPictureChanged)
    Q_PROPERTY(bool arrowsOnHover READ arrowsOnHover WRITE setArrowsOnHover NOTIFY arrowsOnHoverChanged)
    Q_PROPERTY(bool middleClick READ middleClick WRITE setMiddleClick NOTIFY middleClickChanged)
    Q_PROPERTY(int tabBarButtonStyle READ tabBarButtonStyle WRITE setTabBarButtonStyle NOTIFY tabBarButtonStyleChanged)
    Q_PROPERTY(QVariantHash comicData READ comicData NOTIFY comicDataChanged)

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet();

        void init();
        virtual QList<QAction*> contextualActions();

        QGraphicsWidget *graphicsWidget();

        //For QML
        QObject *comicsModel();
        QVariantHash comicData();
        
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
        
        int tabBarButtonStyle() const;
        void setTabBarButtonStyle(int style);
        Q_INVOKABLE bool checkAuthorization(const QString &permissionName) { return hasAuthorization(permissionName); }
        Q_INVOKABLE void showFullView() { fullView(); }
        //End for QML
Q_SIGNALS:
    void showComicUrlChanged();
    void showComicAuthorChanged();
    void showComicTitleChanged();
    void showComicIdentifierChanged();
    void showErrorPictureChanged();
    void arrowsOnHoverChanged();
    void middleClickChanged();
    void tabBarButtonStyleChanged();
    void comicDataChanged();
    void tabHighlightRequest(int index, bool highlight);
    void showNextNewStrip();

    public Q_SLOTS:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
        void createConfigurationInterface( KConfigDialog *parent );

    private Q_SLOTS:
        void slotTabChanged( int newIndex );
        void slotNextDay();
        void slotPreviousDay();
        void slotFirstDay();
        void slotCurrentDay();
        void slotFoundLastStrip( int index, const QString &identifier, const QString &suffix );
        void slotGoJump();
        void slotReload();
        void slotSaveComicAs();
        void slotScaleToContent();
        void slotShop();
        void slotStorePosition();
        void slotSizeChanged();
        void slotShowMaxSize();
        void applyConfig();
        void checkDayChanged();
        void fullView();
        void updateSize();
        void createComicBook();
        void slotArchive( int archiveType, const KUrl &dest, const QString &fromIdentifier, const QString &toIdentifier );
        void slotArchiveFinished( KJob *job );

    public slots:
        void configChanged();
        Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());
        Q_INVOKABLE void goJump() { slotGoJump();}
        Q_INVOKABLE void shop() { slotShop();}
        Q_INVOKABLE void tabChanged(int newIndex) { slotTabChanged(newIndex);}

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    private:
        void changeComic( bool differentComic );
        void updateUsedComics();
        void updateContextMenu();
        void updateView();
        void saveConfig();
        bool isInPanel() const;
        void refreshComicData();
        void setTabHighlighted(int index, bool highlight);
        bool hasHighlightedTabs();
        bool isTabHighlighted(int index) const;

    private:
        static const int CACHE_LIMIT;
        ComicModel *mModel;
        QSortFilterProxyModel *mProxy;
        ActiveComicModel mActiveComicModel;
        QVariantHash mComicData;

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
        int mCheckNewComicStripsIntervall;
        CheckNewStrips *mCheckNewStrips;
        QTimer *mDateChangedTimer;
        QList<QAction*> mActions;
        Plasma::DeclarativeWidget *mDeclarativeWidget;
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
        SavingDir *mSavingDir;
};

K_EXPORT_PLASMA_APPLET( comic, ComicApplet )

#endif
