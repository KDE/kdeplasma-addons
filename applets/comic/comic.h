/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>                     *
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
#include <plasma/applet.h>
#include <plasma/dataengine.h>

class ArrowWidget;
class ConfigWidget;
class FadingItem;
class FullViewWidget;
class ImageWidget;
class QTimer;
class QAction;

namespace Plasma {
class Frame;
class Label;
class PushButton;
class TabBar;
}

class ComicApplet : public Plasma::Applet
{
    Q_OBJECT

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet();

        void init();
        virtual QList<QAction*> contextualActions();

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
        void createLayout();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* );
        void wheelEvent( QGraphicsSceneWheelEvent* );
        void hoverEnterEvent( QGraphicsSceneHoverEvent* );
        void hoverLeaveEvent( QGraphicsSceneHoverEvent* );
        void constraintsEvent( Plasma::Constraints constraints );
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    private:
        void changeComic( bool differentComic );
        void updateComic( const QString &identifierSuffix = QString() );
        void updateTabBar();
        void updateButtons();
        void updateContextMenu();
        void loadConfig();
        void saveConfig();
        void updateSize();

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
        ConfigWidget *mConfigWidget;
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
        FadingItem *mFadingItem;
        Plasma::Label *mLabelId;
        Plasma::Label *mLabelTop;
        Plasma::Label *mLabelUrl;
        Plasma::PushButton *mPrevButton;
        Plasma::PushButton *mNextButton;

        ImageWidget *mImageWidget;
        ArrowWidget *mLeftArrow;
        ArrowWidget *mRightArrow;

        //Tabs
        bool mUseTabs;
        bool mShowTabBar;
        bool mSwitchTabs;
        int mSwitchTabTime;
        Plasma::TabBar *mTabBar;
        QStringList mTabIdentifier;
        QStringList mTabText;
};

K_EXPORT_PLASMA_APPLET( comic, ComicApplet )

#endif
