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

#include <QtCore/QDate>
#include <QtGui/QImage>

#include <KUrl>
#include <Plasma/DataEngine>
#include <Plasma/Label>
#include <Plasma/PopupApplet>
#include <Plasma/TabBar>

class ArrowWidget;
class ComicModel;
class ConfigWidget;
class FullViewWidget;
class ImageWidget;
class QAction;
class QGraphicsLayout;
class QPropertyAnimation;
class QSortFilterProxyModel;
class QTimer;

namespace Plasma {
class Frame;
class PushButton;
}

//Helper class, sets the sizeHint to 0 if the TabBar is hidden
class ComicTabBar : public Plasma::TabBar
{
    public:
        ComicTabBar( QGraphicsWidget *parent = 0 ) : TabBar( parent ) {}
        ~ComicTabBar() {}

        void removeAllTabs()
        {
            while ( this->count() ) {
                this->removeTab( 0 );
            }
        }

    protected:
        QSizeF sizeHint( Qt::SizeHint which, const QSizeF &constraint = QSizeF() ) const
        {
            if ( !isVisible() ) {
                return QSizeF( 0, 0 );
            }
            return QGraphicsWidget::sizeHint( which, constraint );
        }

        void hideEvent( QHideEvent *event )
        {
            updateGeometry();
            QGraphicsWidget::hideEvent( event );
        }

        void showEvent( QShowEvent *event )
        {
            updateGeometry();
            QGraphicsWidget::showEvent( event );
        }
};

//Helper class, sets the sizeHint to 0 if the Label is hidden
class ComicLabel : public Plasma::Label
{
    public:
        ComicLabel( QGraphicsWidget *parent = 0 ) : Plasma::Label( parent ) {}
        ~ComicLabel() {}

    protected:
        QSizeF sizeHint( Qt::SizeHint which, const QSizeF &constraint = QSizeF() ) const
        {
            if ( !isVisible() ) {
                return QSizeF( 0, 0 );
            }
            return QGraphicsProxyWidget::sizeHint( which, constraint );
        }

        void hideEvent( QHideEvent *event )
        {
            updateGeometry();
            QGraphicsProxyWidget::hideEvent( event );
        }

        void showEvent( QShowEvent *event )
        {
            updateGeometry();
            QGraphicsProxyWidget::showEvent( event );
        }
};

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
        void slotGoJump();
        void slotReload();
        void slotStartTimer();
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

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
        bool eventFilter( QObject *receiver, QEvent *event );

    private:
        void changeComic( bool differentComic );
        void updateComic( const QString &identifierSuffix = QString() );
        void updateUsedComics();
        void updateButtons();
        void updateContextMenu();
        void loadConfig();
        void saveConfig();
        bool isInPanel() const;

    private:
        ComicModel *mModel;
        QSortFilterProxyModel *mProxy;

        QImage mImage;
        QDate mCurrentDay;
        KUrl mWebsiteUrl;
        KUrl mShopUrl;
        QString mComicIdentifier;
        QString mNextIdentifierSuffix;
        QString mPreviousIdentifierSuffix;
        QString mFirstIdentifierSuffix;
        QString mComicAuthor;
        QString mComicTitle;
        QString mStripTitle;
        QString mAdditionalText;
        QString mSuffixType;
        QString mShownIdentifierSuffix;
        QString mCurrentIdentifierSuffix;
        QString mStoredIdentifierSuffix;
        QString mIdentifierError;
        QString mSavingDir;
        QString mOldSource;
        ConfigWidget *mConfigWidget;
        bool mDifferentComic;
        bool mScaleComic;
        bool mShowPreviousButton;
        bool mShowNextButton;
        bool mShowComicUrl;
        bool mShowComicAuthor;
        bool mShowComicTitle;
        bool mShowComicIdentifier;
        bool mArrowsOnHover;
        bool mMiddleClick;
        QTimer *mDateChangedTimer;
        QTimer *mReloadTimer;
        QList<QAction*> mActions;
        QGraphicsWidget *mMainWidget;
        FullViewWidget *mFullViewWidget;
        QAction *mActionGoFirst;
        QAction *mActionGoLast;
        QAction *mActionGoJump;
        QAction *mActionScaleContent;
        QAction *mActionShop;
        QAction *mActionStorePosition;
        QMap< QString, int > mFirstStripNum;
        QMap< QString, int > mMaxStripNum;
        QSizeF mMaxSize;
        QSizeF mLastSize;
        QSizeF mIdealSize;
        Plasma::DataEngine *mEngine;
        Plasma::Frame *mFrame;
        QPropertyAnimation *mFrameAnim;
        ComicLabel *mLabelId;
        ComicLabel *mLabelTop;
        ComicLabel *mLabelUrl;
        Plasma::PushButton *mPrevButton;
        Plasma::PushButton *mNextButton;
        Plasma::PushButton *mZoomButton;

        ImageWidget *mImageWidget;
        ArrowWidget *mLeftArrow;
        ArrowWidget *mRightArrow;

        //Tabs
        bool mUseTabs;
        bool mShowTabBar;
        bool mSwitchTabs;
        int mSwitchTabTime;
        ComicTabBar *mTabBar;
        QStringList mTabIdentifier;

        enum TabView {
            ShowText = 0x1,
            ShowIcon = 0x2
        };
        int mTabView;
};

K_EXPORT_PLASMA_APPLET( comic, ComicApplet )

#endif
