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

#include "comic.h"

#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QSortFilterProxyModel>

#include <KConfigDialog>
#include <KDatePicker>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <knuminput.h>
#include <KPushButton>
#include <KRun>
#include <KTemporaryFile>

#include <config-nepomuk.h>
#ifdef HAVE_NEPOMUK
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>

#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/Xesam>
#endif

#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/PushButton>
#include <plasma/tooltipmanager.h>

#include "arrowwidget.h"
#include "comicmodel.h"
#include "configwidget.h"
#include "fullviewwidget.h"
#include "imagewidget.h"

//NOTE based on GotoPageDialog KDE/kdegraphics/okular/part.cpp
//BEGIN choose a strip dialog
class ChooseStripNumDialog : public KDialog
{
    public:
        ChooseStripNumDialog( QWidget *parent, int current, int min, int max )
            : KDialog( parent )
        {
            setCaption( i18n( "Go to Strip" ) );
            setButtons( Ok | Cancel );
            setDefaultButton( Ok );

            QWidget *widget = new QWidget( this );
            setMainWidget( widget );

            QVBoxLayout *topLayout = new QVBoxLayout( widget );
            topLayout->setMargin( 0 );
            topLayout->setSpacing( spacingHint() );
            numInput = new KIntNumInput( current, widget );
            numInput->setRange( min, max );
            numInput->setEditFocus( true );
            numInput->setSliderEnabled( true );

            QLabel *label = new QLabel( i18n( "&Strip Number:" ), widget );
            label->setBuddy( numInput );
            topLayout->addWidget( label );
            topLayout->addWidget( numInput ) ;
            // A little bit extra space
            topLayout->addSpacing( spacingHint() );
            topLayout->addStretch( 10 );
            numInput->setFocus();
        }

        int getStripNumber() const
        {
            return numInput->value();
        }

    protected:
        KIntNumInput *numInput;
};
//END choose a strip dialog

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::PopupApplet( parent, args ),
      mDifferentComic( true ),
      mShowPreviousButton( false ),
      mShowNextButton( false ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false ),
      mArrowsOnHover( true ),
      mMiddleClick( true ),
      mMainWidget( 0 ),
      mFullViewWidget( 0 ),
      mActionShop( 0 ),
      mEngine( 0 ),
      mFrame( 0 ),
      mFrameAnim( 0 ),
      mPrevButton( 0 ),
      mNextButton( 0 ),
      mZoomButton( 0 )
{
    setHasConfigurationInterface( true );
    resize( 600, 250 );
    setAspectRatioMode( Plasma::IgnoreAspectRatio );

    setPopupIcon( "face-smile-big" );

    graphicsWidget();
}

void ComicApplet::init()
{
    connect(this, SIGNAL(appletTransformedByUser()), this, SLOT(slotSizeChanged()));

    Plasma::ToolTipManager::self()->registerWidget( this );

    loadConfig();

    mEngine = dataEngine( "comic" );
    mModel = new ComicModel( mEngine->query( "providers" ), mTabIdentifier, this );
    mProxy = new QSortFilterProxyModel( this );
    mProxy->setSourceModel( mModel );
    mProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
    mProxy->sort( 1, Qt::AscendingOrder );

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, SIGNAL( timeout() ), this, SLOT( checkDayChanged() ) );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes

    mReloadTimer = new QTimer( this );
    slotStartTimer();
    connect( mReloadTimer, SIGNAL( timeout() ), this, SLOT( slotReload() ) );

    mActionGoFirst = new QAction( KIcon( "go-first" ), i18n( "Jump to &first Strip" ), this );
    mActions.append( mActionGoFirst );
    connect( mActionGoFirst, SIGNAL( triggered( bool ) ), this, SLOT( slotFirstDay() ) );

    mActionGoLast = new QAction( KIcon( "go-last" ), i18n( "Jump to &current Strip" ), this );
    mActions.append( mActionGoLast );
    connect( mActionGoLast, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentDay() ) );

    mActionGoJump = new QAction( KIcon( "go-jump" ), i18n( "Jump to Strip ..." ), this );
    mActions.append( mActionGoJump );
    //stop the timer to avoid tab switching while interacting with the applet
    connect( mActionGoJump, SIGNAL( triggered( bool ) ), mReloadTimer, SLOT( stop() ) );
    connect( mActionGoJump, SIGNAL( triggered( bool ) ), this, SLOT( slotGoJump() ) );

    if ( hasAuthorization( "LaunchApp" ) ) {
        mActionShop = new QAction( i18n( "Visit the shop &website" ), this );
        mActionShop->setEnabled( false );
        mActions.append( mActionShop );
        connect( mActionShop, SIGNAL( triggered( bool ) ), this, SLOT( slotShop() ) );
    }

    if ( hasAuthorization( "FileDialog" ) ) {
        QAction *action = new QAction( KIcon( "document-save-as" ), i18n( "&Save Comic As..." ), this );
        mActions.append( action );
        //stop the timer to avoid tab switching while interacting with the applet
        connect( action, SIGNAL( triggered( bool ) ), mReloadTimer, SLOT( stop() ) );
        connect( action, SIGNAL( triggered( bool ) ), this , SLOT( slotSaveComicAs() ) );
    }

    mActionScaleContent = new QAction( KIcon( "zoom-original" ), i18nc( "@option:check Context menu of comic image", "&Actual Size" ), this );
    mActionScaleContent->setCheckable( true );
    mActionScaleContent->setChecked( mScaleComic );
    mActions.append( mActionScaleContent );
    connect( mActionScaleContent, SIGNAL( triggered( bool ) ), this , SLOT( slotScaleToContent() ) );

    mActionStorePosition = new QAction( KIcon( "go-home" ), i18nc( "@option:check Context menu of comic image", "Store current &Position" ), this);
    mActionStorePosition->setCheckable( true );
    mActionStorePosition->setChecked( !mStoredIdentifierSuffix.isEmpty() );
    mActions.append( mActionStorePosition );
    connect( mActionStorePosition, SIGNAL( triggered( bool ) ), this, SLOT( slotStorePosition() ) );

    updateUsedComics();
    changeComic( true );
}

ComicApplet::~ComicApplet()
{
    delete mFullViewWidget;
}

QGraphicsWidget *ComicApplet::graphicsWidget()
{
    if ( !mMainWidget ) {
        mMainWidget = new QGraphicsWidget( this );
        mMainWidget->setMinimumSize( 150, 60 );
        mMainWidget->setAcceptHoverEvents( true );
        mMainWidget->installEventFilter( this );

        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
        layout->setContentsMargins( 0, 0, 0, 0 );
        layout->setSpacing( 0 );

        QGraphicsLinearLayout *centralLayout = new QGraphicsLinearLayout;
        centralLayout->setOrientation( Qt::Vertical );
        centralLayout->setContentsMargins( 0, 0, 0, 0 );
        centralLayout->setSpacing( 0 );

        QGraphicsLinearLayout *bottomLayout = new QGraphicsLinearLayout;
        bottomLayout->setContentsMargins( 0, 2, 0, 0 );
        bottomLayout->setSpacing( 0 );

        mTabBar = new ComicTabBar( mMainWidget );
        mTabBar->nativeWidget()->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
        mTabBar->hide();
        connect( mTabBar, SIGNAL( currentChanged( int ) ), this, SLOT( slotTabChanged( int ) ) );
        centralLayout->addItem( mTabBar );

        mLabelTop = new ComicLabel( mMainWidget );
        mLabelTop->setMinimumWidth( 0 );
        mLabelTop->nativeWidget()->setWordWrap( false );
        mLabelTop->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelTop->setAlignment( Qt::AlignCenter );
        mLabelTop->hide();
        centralLayout->addItem( mLabelTop );
        centralLayout->setItemSpacing( 1, 2 );

        mImageWidget = new ImageWidget( mMainWidget );
        mImageWidget->setZValue( 0 );
        mImageWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
        centralLayout->addItem( mImageWidget );

        mLabelId = new ComicLabel( mMainWidget );
        mLabelId->setMinimumWidth( 0 );
        mLabelId->nativeWidget()->setWordWrap( false );
        mLabelId->nativeWidget()->setCursor( Qt::PointingHandCursor );
        mLabelId->nativeWidget()->setToolTip( i18n( "Jump to Strip ..." ) );
        mLabelId->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelId->hide();
        bottomLayout->addItem( mLabelId );
        bottomLayout->addStretch( 1 );

        mLabelUrl = new ComicLabel( mMainWidget );
        mLabelUrl->setMinimumWidth( 0 );
        mLabelUrl->nativeWidget()->setWordWrap( false );
        if ( hasAuthorization( "LaunchApp" ) ) {
            mLabelUrl->nativeWidget()->setCursor( Qt::PointingHandCursor );
            mLabelUrl->nativeWidget()->setToolTip( i18n( "Visit the comic website" ) );
        }
        mLabelUrl->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelUrl->hide();
        bottomLayout->addItem( mLabelUrl );
        centralLayout->addItem( bottomLayout );

        mLeftArrow = new ArrowWidget( mMainWidget );
        mLeftArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
        mLeftArrow->setCursor( Qt::PointingHandCursor );
        mLeftArrow->hide();
        connect( mLeftArrow, SIGNAL( clicked() ), this, SLOT( slotPreviousDay() ) );
        layout->addItem( mLeftArrow );
        layout->addItem( centralLayout );

        mRightArrow = new ArrowWidget( mMainWidget );
        mRightArrow->setDirection( Plasma::Right );
        mRightArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
        mRightArrow->setCursor( Qt::PointingHandCursor );
        mRightArrow->hide();
        connect( mRightArrow, SIGNAL( clicked() ), this, SLOT( slotNextDay() ) );
        layout->addItem( mRightArrow );

        mMainWidget->setLayout( layout );

    #ifdef HAVE_NEPOMUK
//         for manually saving the comics
        Nepomuk::ResourceManager::instance()->init();
    #endif
    }

    return mMainWidget;
}

void ComicApplet::dataUpdated( const QString&, const Plasma::DataEngine::Data &data )
{
    setBusy( false );
    slotStartTimer();
    if ( data[ "Error" ].toBool() ) {
        if ( !data[ "Previous identifier suffix" ].toString().isEmpty() ) {
            if ( !data[ "Identifier" ].toString().isEmpty() ) {
                mIdentifierError = data[ "Identifier" ].toString();
            }
            updateComic( data[ "Previous identifier suffix" ].toString() );
        } else {
            setConfigurationRequired( true );
        }
        return;
    }

    mImage = data[ "Image" ].value<QImage>();
    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    setAssociatedApplicationUrls(mWebsiteUrl);
    mShopUrl = data[ "Shop Url" ].value<KUrl>();
    mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
    mFirstIdentifierSuffix = data[ "First strip identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mAdditionalText = data[ "Additional text" ].toString();
    mComicAuthor = data[ "Comic Author" ].toString();
    mComicTitle = data[ "Title" ].toString();
    mSuffixType = data[ "SuffixType" ].toString();
    mScaleComic = mActionScaleContent->isChecked();

    // get the text at top
    QString tempTop;
    if ( mShowComicTitle ) {
        tempTop = mComicTitle;
        tempTop += ( ( !mStripTitle.isEmpty() && !mComicTitle.isEmpty() ) ? " - " : "" ) + mStripTitle;
    }
    if ( mShowComicAuthor && !mComicAuthor.isEmpty() ) {
        tempTop = ( !tempTop.isEmpty() ? mComicAuthor + ": " + tempTop : mComicAuthor );
    }

    QString temp = data[ "Identifier" ].toString();
    mCurrentIdentifierSuffix = temp.remove( mComicIdentifier + ':' );

    //call the slot to check if the position needs to be saved
    slotStorePosition();

    KConfigGroup cg = config();
    mShownIdentifierSuffix = "";
    if ( mSuffixType == "Number" ) {
        mShownIdentifierSuffix = i18nc("an abbreviation for Number", "# %1", mCurrentIdentifierSuffix);
        int tempNum = mCurrentIdentifierSuffix.toInt();
        if ( mMaxStripNum[ mComicIdentifier ] < tempNum ) {
            mMaxStripNum[ mComicIdentifier ] = tempNum;
            cg.writeEntry( "maxStripNum_" + mComicIdentifier, mMaxStripNum[ mComicIdentifier ] );
        }

        temp = mFirstIdentifierSuffix.remove( mComicIdentifier + ':' );
        mFirstStripNum[ mComicIdentifier ] = temp.toInt();
    } else if ( mSuffixType == "Date" && QDate::fromString( temp, "yyyy-MM-dd" ).isValid() ) {
        mShownIdentifierSuffix = mCurrentIdentifierSuffix;
    }

    mLabelTop->setText( tempTop );
    mImageWidget->setImage( mImage );
    mImageWidget->setIsLeftToRight( data[ "isLeftToRight" ].toBool() );
    mImageWidget->setIsTopToBottom( data[ "isTopToBottom" ].toBool() );
    mLabelId->setText( mShownIdentifierSuffix );
    mLabelUrl->setText( mWebsiteUrl.host() );
    mImageWidget->setScaled( !mScaleComic );

    mTabBar->setVisible( mShowTabBar && mUseTabs );
    mLabelTop->setVisible( ( mShowComicAuthor || mShowComicTitle ) && !mLabelTop->text().isEmpty() );
    mLabelId->setVisible( mShowComicIdentifier && !mLabelId->text().isEmpty() );
    mLabelUrl->setVisible( mShowComicUrl && !mLabelUrl->text().isEmpty() );

    updateButtons();
    updateContextMenu();

    Plasma::ToolTipContent toolTipData;
    if ( !mAdditionalText.isEmpty() ) {
        toolTipData = Plasma::ToolTipContent( mAdditionalText, QString() );
        toolTipData.setAutohide( false );
    }
    Plasma::ToolTipManager::self()->setContent( mMainWidget, toolTipData );

    if ( !mImage.isNull() ) {
        QTimer::singleShot( 1, this, SLOT( updateSize()) );//HACK
    }

    //prefechtes the previous and following comic for faster navigation
    if ( !mNextIdentifierSuffix.isEmpty() ) {
        mEngine->query( mComicIdentifier + ':' + mNextIdentifierSuffix );
    }

    if ( !mPreviousIdentifierSuffix.isEmpty() ) {
        mEngine->query( mComicIdentifier + ':' + mPreviousIdentifierSuffix );
    }
}

void ComicApplet::createConfigurationInterface( KConfigDialog *parent )
{
    //to not have tab switches while configurating things
    mReloadTimer->stop();

    mConfigWidget = new ConfigWidget( dataEngine( "comic" ), mModel, mTabIdentifier, mProxy, parent );
    mConfigWidget->setShowComicUrl( mShowComicUrl );
    mConfigWidget->setShowComicAuthor( mShowComicAuthor );
    mConfigWidget->setShowComicTitle( mShowComicTitle );
    mConfigWidget->setShowComicIdentifier( mShowComicIdentifier );
    mConfigWidget->setArrowsOnHover( mArrowsOnHover );
    mConfigWidget->setMiddleClick( mMiddleClick );
    QTime time = QTime( mSwitchTabTime / 3600, ( mSwitchTabTime / 60 ) % 60, mSwitchTabTime % 60 );
    mConfigWidget->setTabSwitchTime( time );
    mConfigWidget->setHideTabBar( !mShowTabBar );
    mConfigWidget->setUseTabs( mUseTabs );
    mConfigWidget->setSwitchTabs( mSwitchTabs );
    mConfigWidget->setTabView( mTabView - 1);//-1 because counting starts at 0, yet we use flags that start at 1

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage( mConfigWidget->comicSettings, i18n( "General" ), icon(), i18n( "Press the \"Get New Comics ...\" button to install comics." ) );
    parent->addPage( mConfigWidget->appearanceSettings, i18n( "Appearance" ), "image" );

    connect( mConfigWidget, SIGNAL( maxSizeClicked() ), this, SLOT( slotShowMaxSize() ) );
    connect( parent, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( finished() ), this, SLOT( slotStartTimer() ) );
}

void ComicApplet::applyConfig()
{
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();
    mShowComicIdentifier = mConfigWidget->showComicIdentifier();
    mArrowsOnHover = mConfigWidget->arrowsOnHover();
    mMiddleClick = mConfigWidget->middleClick();
    const QTime time = mConfigWidget->tabSwitchTime();
    mSwitchTabTime = time.second() + time.minute() * 60 + time.hour() * 3600;
    mShowTabBar = !mConfigWidget->hideTabBar();
    mUseTabs = mConfigWidget->useTabs();
    mSwitchTabs = mConfigWidget->switchTabs();
    mTabView = mConfigWidget->tabView() + 1;//+1 because counting starts at 0, yet we use flags that start at 1

    updateUsedComics();
    slotStartTimer();
    saveConfig();
    buttonBar();

    changeComic( mDifferentComic );
}

void ComicApplet::changeComic( bool differentComic )
{
    if ( differentComic ) {
        KConfigGroup cg = config();
        mStoredIdentifierSuffix = cg.readEntry( "storedPosition_" + mComicIdentifier, "" );
        mActionStorePosition->setChecked( !mStoredIdentifierSuffix.isEmpty() );

        // assign mScaleComic the moment the new strip has been loaded (dataUpdated) as up to this point
        // the old one should be still shown with its scaling settings
        bool scaleComic = cg.readEntry( "scaleToContent_" + mComicIdentifier, false );
        mActionScaleContent->setChecked( scaleComic );

        updateComic( mStoredIdentifierSuffix );
    } else {
        updateComic( mCurrentIdentifierSuffix );
    }
}

void ComicApplet::updateUsedComics()
{
    const QString oldIdentifier = mComicIdentifier;
    mTabBar->removeAllTabs();
    mTabIdentifier.clear();
    mComicIdentifier.clear();
    mComicTitle.clear();

    bool isFirst = true;
    QModelIndex data;
    for ( int i = 0; i < mProxy->rowCount(); ++i ) {
        if ( mProxy->index( i, 0 ).data( Qt::CheckStateRole) == Qt::Checked ) {
            data = mProxy->index( i, 1 );

            if ( isFirst ) {
                isFirst = false;
                mComicIdentifier = data.data( Qt::UserRole ).toString();
                mDifferentComic = ( oldIdentifier != mComicIdentifier );
                mComicTitle = data.data().toString();
            }

            QIcon icon;
            QString name;
            if ( mTabView & ShowText ) {
                name = data.data().toString();
            }
            if ( mTabView & ShowIcon ) {
                icon = data.data( Qt::DecorationRole ).value<QIcon>();
            }

            mTabBar->addTab( icon, name );
            mTabIdentifier << data.data( Qt::UserRole ).toString();
        }
    }
}

void ComicApplet::slotTabChanged( int newIndex )
{
    if ( newIndex >= mTabIdentifier.count() ) {
        return;
    }

    bool differentComic = ( mComicIdentifier != mTabIdentifier.at( newIndex ) );
    mComicIdentifier = mTabIdentifier.at( newIndex );
    changeComic( differentComic );
}

void ComicApplet::checkDayChanged()
{
    if ( ( mCurrentDay != QDate::currentDate() ) || mImage.isNull() )
        updateComic( mStoredIdentifierSuffix );

    mCurrentDay = QDate::currentDate();
}

void ComicApplet::loadConfig()
{
    KConfigGroup cg = config();
    mTabIdentifier = cg.readEntry( "tabIdentifier", QStringList( QString() ) );
    mComicIdentifier = mTabIdentifier.count() ? mTabIdentifier.at( 0 ) : QString();
    mShowComicUrl = cg.readEntry( "showComicUrl", false );
    mShowComicAuthor = cg.readEntry( "showComicAuthor", false );
    mShowComicTitle = cg.readEntry( "showComicTitle", false );
    mShowComicIdentifier = cg.readEntry( "showComicIdentifier", false );
    mArrowsOnHover = cg.readEntry( "arrowsOnHover", true );
    mMiddleClick = cg.readEntry( "middleClick", true );
    mScaleComic = cg.readEntry( "scaleToContent_" + mComicIdentifier, false );
    mMaxStripNum[ mComicIdentifier ] = cg.readEntry( "maxStripNum_" + mComicIdentifier, 0 );
    mStoredIdentifierSuffix = cg.readEntry( "storedPosition_" + mComicIdentifier, QString() );

    //use a decent default size
    const QSizeF tempMaxSize = isInPanel() ? QSizeF( 600, 250 ) : this->size();
    mMaxSize = cg.readEntry( "maxSize", tempMaxSize );
    mLastSize = mMaxSize;

    mSwitchTabTime = cg.readEntry( "switchTabTime", 10 );// 10 seconds as default
    mShowTabBar = cg.readEntry( "showTabBar", true );
    mUseTabs = cg.readEntry( "useTabs", false );
    mSwitchTabs = cg.readEntry( "switchTabs", false );
    mTabView = cg.readEntry( "tabView", ShowText | ShowIcon );
    mSavingDir = cg.readEntry( "savingDir", QString() );
    mOldSource = mComicIdentifier + ':' + mStoredIdentifierSuffix;

    buttonBar();
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
    cg.writeEntry( "showComicUrl", mShowComicUrl );
    cg.writeEntry( "showComicAuthor", mShowComicAuthor );
    cg.writeEntry( "showComicTitle", mShowComicTitle );
    cg.writeEntry( "showComicIdentifier", mShowComicIdentifier );
    cg.writeEntry( "arrowsOnHover", mArrowsOnHover );
    cg.writeEntry( "middleClick", mMiddleClick );
    cg.writeEntry( "switchTabTime", mSwitchTabTime );
    cg.writeEntry( "showTabBar", mShowTabBar );
    cg.writeEntry( "tabIdentifier", mTabIdentifier );
    cg.writeEntry( "useTabs", mUseTabs );
    cg.writeEntry( "switchTabs", mSwitchTabs );
    cg.writeEntry( "tabView", mTabView );
    cg.writeEntry( "savingDir", mSavingDir );
}

void ComicApplet::slotChosenDay( const QDate &date )
{
    if ( mSuffixType == "Date" ) {
        if ( date <= mCurrentDay ) {
            QDate temp = QDate::fromString( mFirstIdentifierSuffix, "yyyy-MM-dd" );
            if ( temp.isValid() && date >= temp ) {
                updateComic( date.toString( "yyyy-MM-dd" ) );
                // even update if there is not first day identifierSuffix
            } else if ( !temp.isValid() ) {
                updateComic( date.toString( "yyyy-MM-dd" ) );
            }
        }
    }
}

void ComicApplet::slotNextDay()
{
    updateComic( mNextIdentifierSuffix );
}

void ComicApplet::slotPreviousDay()
{
    updateComic( mPreviousIdentifierSuffix );
}

void ComicApplet::slotFirstDay()
{
    updateComic( mFirstIdentifierSuffix );
}

void ComicApplet::slotCurrentDay()
{
    updateComic( QString() );
}

void ComicApplet::slotReload()
{
    int index = mTabBar->currentIndex();
    int newIndex = ( index + 1 ) % mTabBar->count();

    if ( index == newIndex ) {
        changeComic( false );
    } else {
        mTabBar->setCurrentIndex( newIndex );
    }
}

void ComicApplet::slotStartTimer()
{
    if ( mUseTabs && mSwitchTabTime && mSwitchTabs && mTabIdentifier.count() > 1 ) {
        mReloadTimer->start( mSwitchTabTime * 1000 );
    } else {
        mReloadTimer->stop();
    }
}

void ComicApplet::slotGoJump()
{
    if ( mSuffixType == "Number" ) {
        QPointer<ChooseStripNumDialog> pageDialog = new ChooseStripNumDialog( 0, mCurrentIdentifierSuffix.toInt(), mFirstStripNum[ mComicIdentifier ], mMaxStripNum[ mComicIdentifier ] );
        if ( pageDialog->exec() == QDialog::Accepted ) {
            updateComic( QString::number( pageDialog->getStripNumber() ) );
        }
        delete pageDialog;
        slotStartTimer();
    } else if ( mSuffixType == "Date" ) {
        KDatePicker *calendar = new KDatePicker;
        calendar->setAttribute( Qt::WA_DeleteOnClose );//to have destroyed emitted upon closing
        calendar->setMinimumSize( calendar->sizeHint() );
        calendar->setDate( QDate::fromString( mCurrentIdentifierSuffix, "yyyy-MM-dd" ) );

        connect( calendar, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotStartTimer() ) );
        connect( calendar, SIGNAL( dateSelected( QDate ) ), this, SLOT( slotChosenDay( QDate ) ) );
        connect( calendar, SIGNAL( dateEntered( QDate ) ), this, SLOT( slotChosenDay( QDate ) ) );
        calendar->show();
    }
}

void ComicApplet::slotStorePosition()
{
    KConfigGroup cg = config();
    mStoredIdentifierSuffix = mActionStorePosition->isChecked() ? mCurrentIdentifierSuffix : "";
    cg.writeEntry( "storedPosition_" + mComicIdentifier, mStoredIdentifierSuffix );
}

void ComicApplet::slotSizeChanged()
{
    // if the applet was resized manually by the user
    if ( isInPanel() ) {
        if ( mMainWidget->size() != mLastSize ) {
            mMaxSize = mMainWidget->size();
            updateSize();

            KConfigGroup cg = config();
            cg.writeEntry( "maxSize", mMaxSize );
        }
    } else if ( this->geometry().size() != mLastSize ) {
        mMaxSize = this->geometry().size();
        updateSize();

        KConfigGroup cg = config();
        cg.writeEntry( "maxSize", mMaxSize );
    }
}

void ComicApplet::slotShowMaxSize()
{
    resize( mMaxSize );
    emit appletTransformedItself();
}

void ComicApplet::slotShop()
{
    KRun::runUrl( mShopUrl, "text/html", 0 );
}

bool ComicApplet::isInPanel() const
{
    return ( this->geometry().width() < 70 ) || ( this->geometry().height() < 50 );
}

void ComicApplet::updateSize()
{
    if ( configurationRequired() ) {
        return;
    }

    QSizeF notAvailableSize;
    if ( isInPanel() ) {
        notAvailableSize =  mMainWidget->geometry().size() - mImageWidget->size();
    } else {
        notAvailableSize =  this->geometry().size() - mImageWidget->size();
    }
    QSizeF availableSize = mMaxSize - notAvailableSize;
    mImageWidget->setAvailableSize( availableSize );
    mLastSize = mImageWidget->preferredSize() + notAvailableSize;

    if ( isInPanel() ) {
        mMainWidget->resize( mLastSize );
    } else {
        resize( mLastSize );
        mLastSize = this->size();//NOTE the applet won't be smaller than the minimum size of the MainWidget, thus the result of the resize might not correspond with mLastSize
        emit sizeHintChanged( Qt::PreferredSize );
        emit appletTransformedItself();
    }
    mImageWidget->update();
}

QList<QAction*> ComicApplet::contextualActions()
{
    return mActions;
}

QSizeF ComicApplet::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which != Qt::PreferredSize) {
        return Applet::sizeHint(which, constraint);
    } else {
        QSize imageSize = mImage.size();
        if (!imageSize.isEmpty()) {
            return imageSize;
        } else {
            return Applet::sizeHint(which, constraint);
        }
    }
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    //always stop the timer when changing the comic, as not all clicks are caught by mousePressEvent here
    mReloadTimer->stop();

    mEngine = dataEngine( "comic" );

    setConfigurationRequired( mComicIdentifier.isEmpty() );
    if ( !mComicIdentifier.isEmpty() && mEngine && mEngine->isValid() ) {
        setBusy( true );
        const QString identifier = mComicIdentifier + ':' + identifierSuffix;

        //disconnecting of the oldSource is needed, otherwise you could get data for comics you are not looking at if you use tabs
        //if there was an error only disconnect the oldSource if it had nothing to do with the error or if the comic changed, that way updates of the error can come in
        if ( !mIdentifierError.isEmpty() && !mIdentifierError.contains( mComicIdentifier ) ) {
            mEngine->disconnectSource( mIdentifierError, this );
            mIdentifierError.clear();
        }
        if ( ( mIdentifierError != mOldSource ) && ( mOldSource != identifier ) ) {
            mEngine->disconnectSource( mOldSource, this );
        }
        mOldSource = identifier;

        mEngine->disconnectSource( identifier, this );
        mEngine->connectSource( identifier, this );
        const Plasma::DataEngine::Data data = mEngine->query( identifier );

        if ( data[ "Error" ].toBool() && data[ "Previous identifier suffix" ].toString().isEmpty() ) {
            setBusy( false );
            setConfigurationRequired( true );
            slotStartTimer();
        }
    } else {
        setConfigurationRequired( true );
        slotStartTimer();
    }
}

void ComicApplet::updateButtons()
{
    mShowNextButton = !mNextIdentifierSuffix.isEmpty();
    mShowPreviousButton = !mPreviousIdentifierSuffix.isEmpty();

    mLeftArrow->setVisible( !mArrowsOnHover && mShowPreviousButton );
    mRightArrow->setVisible( !mArrowsOnHover && mShowNextButton );

    if ( mNextButton && mPrevButton ) {
        mNextButton->setEnabled( mShowNextButton );
        mPrevButton->setEnabled( mShowPreviousButton );
    }
}

void ComicApplet::updateContextMenu()
{
    mActionGoFirst->setVisible( !mFirstIdentifierSuffix.isEmpty() );
    mActionGoFirst->setEnabled( !mPreviousIdentifierSuffix.isEmpty() );
    mActionGoLast->setEnabled( true );//always enable to have some kind of refresh action
    if (mActionShop) {
        mActionShop->setEnabled( mShopUrl.isValid() );
    }
    mActionGoJump->setEnabled( mSuffixType != "String" );
}

void ComicApplet::slotSaveComicAs()
{
    KTemporaryFile tempFile;

    if ( !tempFile.open() ) {
        slotStartTimer();
        return;
    }

    // save image to temporary file
    mImage.save( tempFile.fileName(), "PNG" );

    KUrl srcUrl( tempFile.fileName() );

    QString dir = mSavingDir;
    if ( dir.isEmpty() ) {
        dir = KGlobalSettings::picturesPath();
    }
    if ( dir.isEmpty() ) {
        dir = KGlobalSettings::downloadPath();
    }
    if ( dir.isEmpty() ) {
        dir = QDir::homePath();
    }

    QString name = mComicTitle + " - " + mCurrentIdentifierSuffix + ".png";

    KUrl destUrl = KUrl(dir);
    destUrl.addPath( name );

    destUrl = KFileDialog::getSaveUrl( destUrl, "*.png" );
    if ( !destUrl.isValid() ) {
        slotStartTimer();
        return;
    }

    mSavingDir = destUrl.directory();
    saveConfig();

#ifdef HAVE_NEPOMUK
    bool worked = KIO::NetAccess::file_copy( srcUrl, destUrl );
    //store additional data using Nepomuk
    if ( worked ) {
        Nepomuk::Resource res( destUrl );
        Nepomuk::Tag tag( i18n( "Comic" ) );

        tag.setLabel( i18n( "Comic" ) );
        res.addTag( tag );
        if ( !mAdditionalText.isEmpty() ) {
            res.setDescription( mAdditionalText );
        }
        if ( !mComicAuthor.isEmpty() ) {
            res.setProperty( Soprano::Vocabulary::NAO::creator(), Nepomuk::Variant( mComicAuthor ) );
        }
        if ( ( mSuffixType == "Date" ) && !mShownIdentifierSuffix.isEmpty() ) {
            res.setProperty( Soprano::Vocabulary::NAO::created(), Nepomuk::Variant( QDate::fromString( mShownIdentifierSuffix, Qt::ISODate ) ) );
        }
        if ( !mComicTitle.isEmpty() ) {
            res.setProperty( Soprano::Vocabulary::Xesam::subject(), Nepomuk::Variant( mComicTitle ) );
        }
        if ( !mStripTitle.isEmpty() ) {
            res.setProperty( Soprano::Vocabulary::Xesam::title(), Nepomuk::Variant( mStripTitle ) );
        }
    }
#else
    KIO::NetAccess::file_copy( srcUrl, destUrl );
#endif

    slotStartTimer();
}

bool ComicApplet::eventFilter( QObject *receiver, QEvent *event )
{
    if ( dynamic_cast<QGraphicsWidget *>(receiver) != mMainWidget ) {
        return false;
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverLeave:
            if ( mArrowsOnHover && mFrameAnim ) {
                mFrameAnim->setDirection( QAbstractAnimation::Backward );
                mFrameAnim->start();
            }

            break;
        case QEvent::GraphicsSceneHoverEnter:
            if ( !configurationRequired() && mArrowsOnHover && mFrameAnim ) {
                mFrameAnim->setDirection( QAbstractAnimation::Forward );
                mFrameAnim->start();
            }

            break;
        case QEvent::GraphicsSceneWheel:
            {
                slotStartTimer();
                QGraphicsSceneWheelEvent *e = static_cast<QGraphicsSceneWheelEvent *>( event );
                if ( mImageWidget->isUnderMouse() && ( e->modifiers() == Qt::ControlModifier ) ) {
                    const QPointF eventPos = e->pos();
                    const int numDegrees = e->delta() / 8;
                    const int numSteps = numDegrees / 15;

                    int index = mTabBar->currentIndex();
                    int count = mTabBar->count();
                    int newIndex = 0;

                    if ( numSteps % count != 0 ) {
                        if ( numSteps < 0 ) {
                            newIndex = ( index - numSteps ) % count;
                        } else if ( numSteps > 0 ) {
                            newIndex = index - ( numSteps % count );
                            newIndex = newIndex < 0 ? newIndex + count : newIndex;
                        }
                        mTabBar->setCurrentIndex( newIndex );
                    }
                    e->accept();
                }
            }
            break;
        case QEvent::GraphicsSceneMousePress:
            {
                slotStartTimer();
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>( event );
                if ( e->button() == Qt::LeftButton ) {
                    if ( mLabelUrl->isUnderMouse() ) {
                        if ( hasAuthorization( "LaunchApp" ) ) {
                            // link clicked
                            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
                        }
                    } else if ( mLabelId->isUnderMouse() ) {
                        // identifierSuffix clicked clicked
                        slotGoJump();
                    } else if ( mImageWidget->isUnderMouse() && ( mMainWidget->geometry().size() != mLastSize ) ) {
                        // only update the size by clicking on the image-rect if the user manual resized the applet
                        updateSize();
                    }
                } else if ( ( e->button() == Qt::MidButton ) && mMiddleClick ) { // handle full view
                    fullView();
                }

                e->ignore();
            }
            break;
        case QEvent::GraphicsSceneResize:
            if ( mFrame ) {
                QPointF buttons( ( mMainWidget->size().width() - mFrame->size().width() ) / 2,
                                mMainWidget->contentsRect().bottom() - mFrame->size().height() - 5 );
                mFrame->setPos( buttons );
            }

            break;
        default:
            break;
    }

    return false;
}

void ComicApplet::slotScaleToContent()
{
    mScaleComic = mActionScaleContent->isChecked();
    mImageWidget->setScaled( !mScaleComic );

    KConfigGroup cg = config();
    cg.writeEntry( "scaleToContent_" + mComicIdentifier,  mScaleComic );

    updateSize();
}

void ComicApplet::buttonBar()
{
    if ( mArrowsOnHover ) {
        if ( !mFrame ) {
            mFrame = new Plasma::Frame( mMainWidget );
            mFrame->setZValue( 10 );
            QGraphicsLinearLayout *l = new QGraphicsLinearLayout();
            mPrevButton = new Plasma::PushButton( mFrame );
            mPrevButton->nativeWidget()->setIcon( KIcon( "arrow-left" ) );
            mPrevButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mPrevButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mPrevButton, SIGNAL( clicked() ), this , SLOT( slotPreviousDay() ) );
            l->addItem( mPrevButton );

            mZoomButton = new Plasma::PushButton( mFrame );
            mZoomButton->nativeWidget()->setIcon( KIcon( "zoom-original" ) );
            mZoomButton->nativeWidget()->setToolTip( i18n( "Show at actual size in a different view.  Alternatively, click with the middle mouse button on the comic." ) );
            mZoomButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mZoomButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mZoomButton, SIGNAL( clicked() ), this, SLOT( fullView() ) );
            l->addItem( mZoomButton );

            mNextButton = new Plasma::PushButton( mFrame );
            mNextButton->nativeWidget()->setIcon( KIcon( "arrow-right" ) );
            mNextButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mNextButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mNextButton, SIGNAL( clicked() ), this , SLOT( slotNextDay() ) );
            l->addItem( mNextButton );
            mFrame->setLayout( l );
            mFrame->setFrameShadow( Plasma::Frame::Raised );
            // To get correct frame size in constraintsEvent
            l->activate();
            mFrame->setOpacity( 0.0 );

            mFrameAnim = new QPropertyAnimation( mFrame, "opacity", mFrame );
            mFrameAnim->setDuration( 100 );
            mFrameAnim->setStartValue( 0.0 );
            mFrameAnim->setEndValue( 1.0 );

            // Set frame position
            constraintsEvent( Plasma::SizeConstraint );
        }
    } else {
        delete mFrame;
        mFrame = 0;
        mFrameAnim = 0;
        mPrevButton = 0;
        mNextButton = 0;
    }
}

void ComicApplet::fullView()
{
    if ( !mFullViewWidget ) {
        mFullViewWidget = new FullViewWidget();
    }

    if ( !mFullViewWidget->isVisible() ) {
        mFullViewWidget->setImage( mImage );
        foreach (QGraphicsView *view, scene()->views()) {
            if (view->sceneRect().contains(mMainWidget->pos())) {
                QPoint viewPos = view->pos();
                QPoint relPos = mapToScene( 0, 0 ).toPoint();
                mFullViewWidget->adaptPosition( relPos + view->mapToGlobal(viewPos) );
                break;
            }
        }
        mFullViewWidget->show();
    }
}

#include "comic.moc"
