/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2011 Matthias Fuchs <mat69@gmx.net>                *
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
#include "comicarchivedialog.h"
#include "comicarchivejob.h"
#include "checknewstrips.h"

#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QSortFilterProxyModel>

#include <KAction>
#include <KConfigDialog>
#include <KDatePicker>
#include <KDebug>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KInputDialog>
#include <KIO/NetAccess>
#include <KNotification>
#include <kuiserverjobtracker.h>
#include <knuminput.h>
#include <KPushButton>
#include <KRun>
#include <KStandardShortcut>
#include <KTemporaryFile>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NFO>
#include <Nepomuk/Vocabulary/PIMO>
#include <nepomuk/utils.h>

using namespace Nepomuk::Vocabulary;
#endif

#include <Plasma/Containment>
#include <Plasma/Frame>
#include <Plasma/PushButton>
#include <Plasma/Theme>
#include <plasma/tooltipmanager.h>

#include "arrowwidget.h"
#include "comicmodel.h"
#include "configwidget.h"
#include "fullviewwidget.h"
#include "imagewidget.h"

K_GLOBAL_STATIC( ComicUpdater, globalComicUpdater )

const int ComicApplet::CACHE_LIMIT = 20;

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
      mShowErrorPicture( true ),
      mArrowsOnHover( true ),
      mMiddleClick( true ),
      mCheckNewStrips( 0 ),
      mMainWidget( 0 ),
      mCentralLayout( 0 ),
      mBottomLayout( 0 ),
      mFullViewWidget( 0 ),
      mActionShop( 0 ),
      mEngine( 0 ),
      mFrame( 0 ),
      mFrameAnim( 0 ),
      mPrevButton( 0 ),
      mNextButton( 0 ),
      mZoomButton( 0 ),
      mTabAdded( false )
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

    globalComicUpdater->init( globalConfig() );
    configChanged();
    
    buttonBar();
    
    mEngine = dataEngine( "comic" );
    mModel = new ComicModel( mEngine->query( "providers" ), mTabIdentifier, this );
    mProxy = new QSortFilterProxyModel( this );
    mProxy->setSourceModel( mModel );
    mProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
    mProxy->sort( 1, Qt::AscendingOrder );

    //set maximum number of cached strips per comic, -1 means that there is no limit
    KConfigGroup global = globalConfig();
    //convert old values
    if ( global.hasKey( "useMaxComicLimit" ) ) {
        const bool use = global.readEntry( "useMaxComicLimit", false );
        if ( !use ) {
            global.writeEntry( "maxComicLimit", 0 );
        }
        global.deleteEntry( "useMaxComicLimit" );
    }
    const int maxComicLimit = global.readEntry( "maxComicLimit", CACHE_LIMIT );
    mEngine->query( QLatin1String( "setting_maxComicLimit:" ) + QString::number( maxComicLimit ) );

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, SIGNAL(timeout()), this, SLOT(checkDayChanged()) );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes

    mActionNextNewStripTab = new KAction( KIcon( "go-next-view" ), i18nc( "here strip means comic strip", "&Next Tab with a new Strip" ), this );
    mActionNextNewStripTab->setShortcut( KStandardShortcut::openNew() );
    addAction( "next new strip" , mActionNextNewStripTab );
    mActions.append( mActionNextNewStripTab );
    connect( mActionNextNewStripTab, SIGNAL(triggered(bool)), this, SLOT(slotNextNewStrip()) );

    mActionGoFirst = new QAction( KIcon( "go-first" ), i18n( "Jump to &first Strip" ), this );
    mActions.append( mActionGoFirst );
    connect( mActionGoFirst, SIGNAL(triggered(bool)), this, SLOT(slotFirstDay()) );

    mActionGoLast = new QAction( KIcon( "go-last" ), i18n( "Jump to &current Strip" ), this );
    mActions.append( mActionGoLast );
    connect( mActionGoLast, SIGNAL(triggered(bool)), this, SLOT(slotCurrentDay()) );

    mActionGoJump = new QAction( KIcon( "go-jump" ), i18n( "Jump to Strip ..." ), this );
    mActions.append( mActionGoJump );
    connect( mActionGoJump, SIGNAL(triggered(bool)), this, SLOT(slotGoJump()) );

    if ( hasAuthorization( "LaunchApp" ) ) {
        mActionShop = new QAction( i18n( "Visit the shop &website" ), this );
        mActionShop->setEnabled( false );
        mActions.append( mActionShop );
        connect( mActionShop, SIGNAL(triggered(bool)), this, SLOT(slotShop()) );
    }

    if ( hasAuthorization( "FileDialog" ) ) {
        QAction *action = new QAction( KIcon( "document-save-as" ), i18n( "&Save Comic As..." ), this );
        mActions.append( action );
        connect( action, SIGNAL(triggered(bool)), this , SLOT(slotSaveComicAs()) );
    }

    if ( hasAuthorization( "FileDialog" ) ) {
        QAction *action = new QAction( KIcon( "application-epub+zip" ), i18n( "&Create Comic Book Archive..." ), this );
        mActions.append( action );
        connect( action, SIGNAL(triggered(bool)), this, SLOT(createComicBook()) );
    }

    mActionScaleContent = new QAction( KIcon( "zoom-original" ), i18nc( "@option:check Context menu of comic image", "&Actual Size" ), this );
    mActionScaleContent->setCheckable( true );
    mActionScaleContent->setChecked( mScaleComic );
    mActions.append( mActionScaleContent );
    connect( mActionScaleContent, SIGNAL(triggered(bool)), this , SLOT(slotScaleToContent()) );

    mActionStorePosition = new QAction( KIcon( "go-home" ), i18nc( "@option:check Context menu of comic image", "Store current &Position" ), this);
    mActionStorePosition->setCheckable( true );
    mActionStorePosition->setChecked( !mStoredIdentifierSuffix.isEmpty() );
    mActions.append( mActionStorePosition );
    connect( mActionStorePosition, SIGNAL(triggered(bool)), this, SLOT(slotStorePosition()) );

    //make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

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

        mCentralLayout = new QGraphicsLinearLayout;
        mCentralLayout->setOrientation( Qt::Vertical );
        mCentralLayout->setContentsMargins( 0, 0, 0, 0 );
        mCentralLayout->setSpacing( 0 );

        mBottomLayout = new QGraphicsLinearLayout;
        mBottomLayout->setContentsMargins( 0, 2, 0, 0 );
        mBottomLayout->setSpacing( 0 );

        mTabBar = new ComicTabBar( mMainWidget );
        mTabBar->nativeWidget()->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
        mTabBar->hide();
        connect( mTabBar, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)) );

        mLabelTop = new ComicLabel( mMainWidget );
        mLabelTop->setMinimumWidth( 0 );
        mLabelTop->nativeWidget()->setWordWrap( false );
        mLabelTop->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelTop->setAlignment( Qt::AlignCenter );
        mLabelTop->hide();
        mCentralLayout->addItem( mLabelTop );

        mImageWidget = new ImageWidget( mMainWidget );
        mImageWidget->setZValue( 0 );
        mImageWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
        mCentralLayout->addItem( mImageWidget );

        mLabelId = new ComicLabel( mMainWidget );
        mLabelId->setMinimumWidth( 0 );
        mLabelId->nativeWidget()->setWordWrap( false );
        mLabelId->nativeWidget()->setCursor( Qt::PointingHandCursor );
        mLabelId->nativeWidget()->setToolTip( i18n( "Jump to Strip ..." ) );
        mLabelId->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelId->hide();
        mBottomLayout->addItem( mLabelId );
        mBottomLayout->addStretch( 1 );

        mLabelUrl = new ComicLabel( mMainWidget );
        mLabelUrl->setMinimumWidth( 0 );
        mLabelUrl->nativeWidget()->setWordWrap( false );
        if ( hasAuthorization( "LaunchApp" ) ) {
            mLabelUrl->nativeWidget()->setCursor( Qt::PointingHandCursor );
            mLabelUrl->nativeWidget()->setToolTip( i18n( "Visit the comic website" ) );
        }
        mLabelUrl->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
        mLabelUrl->hide();
        mBottomLayout->addItem( mLabelUrl );
        mCentralLayout->addItem( mBottomLayout );

        mLeftArrow = new ArrowWidget( mMainWidget );
        mLeftArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
        mLeftArrow->setCursor( Qt::PointingHandCursor );
        mLeftArrow->hide();
        connect( mLeftArrow, SIGNAL(clicked()), this, SLOT(slotPreviousDay()) );
        layout->addItem( mLeftArrow );
        layout->addItem( mCentralLayout );

        mRightArrow = new ArrowWidget( mMainWidget );
        mRightArrow->setDirection( Plasma::Right );
        mRightArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
        mRightArrow->setCursor( Qt::PointingHandCursor );
        mRightArrow->hide();
        connect( mRightArrow, SIGNAL(clicked()), this, SLOT(slotNextDay()) );
        layout->addItem( mRightArrow );

        mMainWidget->setLayout( layout );

    #ifdef HAVE_NEPOMUK
//         for manually saving the comics
        Nepomuk::ResourceManager::instance()->init();
    #endif
    }

    return mMainWidget;
}

void ComicApplet::setTabBarVisible( bool isVisible )
{
    if ( !mCentralLayout ) {
        return;
    }

    mTabBar->setVisible( isVisible );
    if ( !mTabAdded && isVisible ) {
        mCentralLayout->insertItem( 0, mTabBar );
        mTabAdded = true;
    } else if ( mTabAdded && !isVisible ) {
        mCentralLayout->removeItem( mTabBar );
        mTabAdded = false;
    }
}

void ComicApplet::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    //disconnect prefetched comic strips
    if ( source != mOldSource ) {
        mEngine->disconnectSource( source, this );
        return;
    }

    setBusy( false );
    setConfigurationRequired( false );

    //there was an error, display information as image
    const bool hasError = data[ "Error" ].toBool();
    const bool errorAutoFixable = data[ "Error automatically fixable" ].toBool();
    if ( hasError ) {
        const QString previousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
        if ( !mShowErrorPicture && !previousIdentifierSuffix.isEmpty() ) {
            mEngine->disconnectSource( source, this );
            updateComic( previousIdentifierSuffix );
            return;
        }
        QPixmap errorPic( 500, 400 );
        errorPic.fill();
        QPainter p( &errorPic );
        QFont font = Plasma::Theme::defaultTheme()->font( Plasma::Theme::DefaultFont );
        font.setPointSize( 24 );
        p.setPen( QColor( 0, 0, 0 ) );
        p.setFont( font );
        QString title = i18n( "Getting comic strip failed:" );
        p.drawText( QRect( 10, 10 , 480, 100 ), Qt::TextWordWrap | Qt::AlignHCenter | Qt::AlignVCenter, title );
        QString text = i18n( "Maybe there is no Internet connection.\nMaybe the comic plugin is broken.\nAnother reason might be that there is no comic for this day/number/string, so choosing a different one might work." );

        mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
        mNextIdentifierSuffix.clear();
        if ( !mPreviousIdentifierSuffix.isEmpty() ) {
            if ( !data[ "Identifier" ].toString().isEmpty() ) {
                mIdentifierError = data[ "Identifier" ].toString();
            }
            text.append( i18n( "\n\nChoose the previous strip to go to the last cached strip." ) );
        }

        font.setPointSize( 16 );
        p.setFont( font );
        p.drawText( QRect( 10, 120 , 480, 270 ), Qt::TextWordWrap | Qt::AlignLeft, text );

        mImage = errorPic.toImage();
        mAdditionalText = title + text;
    } else {
        mImage = data[ "Image" ].value<QImage>();
        mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
        mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
        mAdditionalText = data[ "Additional text" ].toString();
    }

    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    setAssociatedApplicationUrls(mWebsiteUrl);
    mImageUrl = data["Image Url"].value<KUrl>();
    mShopUrl = data[ "Shop Url" ].value<KUrl>();
    mFirstIdentifierSuffix = data[ "First strip identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mComicAuthor = data[ "Comic Author" ].toString();
    mComicTitle = data[ "Title" ].toString();
    mScaleComic = mActionScaleContent->isChecked();

    const QString suffixType = data[ "SuffixType" ].toString();
    if ( suffixType == "Date" ) {
        mComicType = Date;
    } else if ( suffixType == "Number" ) {
        mComicType = Number;
    } else {
        mComicType = String;
    }

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

    //looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if ( mNextIdentifierSuffix.isEmpty() || ( mCurrentIdentifierSuffix == mLastIdentifierSuffix ) ) {
        cg.writeEntry( "lastStripVisited_" + mComicIdentifier, true );
        const int index = mTabBar->currentIndex();
        if ( mCheckNewComicStripsIntervall ) {
            mTabBar->setTabHighlighted( index, false );
            mActionNextNewStripTab->setEnabled( mTabBar->hasHighlightedTabs() );
        }
    }

    //found a new last identifier
    if ( mNextIdentifierSuffix.isEmpty() ) {
        mLastIdentifierSuffix = mCurrentIdentifierSuffix;
        cg.writeEntry( "lastStrip_" + mComicIdentifier, mCurrentIdentifierSuffix );
    }

    //call the slot to check if the position needs to be saved
    slotStorePosition();

    mShownIdentifierSuffix = "";
    if ( mComicType == Number ) {
        mShownIdentifierSuffix = i18nc("an abbreviation for Number", "# %1", mCurrentIdentifierSuffix);
        int tempNum = mCurrentIdentifierSuffix.toInt();
        if ( mMaxStripNum[ mComicIdentifier ] < tempNum ) {
            mMaxStripNum[ mComicIdentifier ] = tempNum;
            cg.writeEntry( "maxStripNum_" + mComicIdentifier, mMaxStripNum[ mComicIdentifier ] );
        }

        temp = mFirstIdentifierSuffix.remove( mComicIdentifier + ':' );
        mFirstStripNum[ mComicIdentifier ] = temp.toInt();
    } else if ( mComicType == Date && QDate::fromString( temp, "yyyy-MM-dd" ).isValid() ) {
        mShownIdentifierSuffix = mCurrentIdentifierSuffix;
    } else if ( mComicType == String ) {
        mShownIdentifierSuffix = mCurrentIdentifierSuffix;
    }

    mLabelTop->setText( tempTop );
    mImageWidget->setImage( mImage );
    mImageWidget->setIsLeftToRight( data[ "isLeftToRight" ].toBool() );
    mImageWidget->setIsTopToBottom( data[ "isTopToBottom" ].toBool() );
    mLabelId->setText( mShownIdentifierSuffix );
    mLabelUrl->setText( mWebsiteUrl.host() );
    mImageWidget->setScaled( !mScaleComic );

    //disconnect if there is either no error, or an error that can not be fixed automatically 
    if ( !errorAutoFixable ) {
        mEngine->disconnectSource( source, this );
    }

    //prefetch the previous and following comic for faster navigation
    if ( !mNextIdentifierSuffix.isEmpty() ) {
        const QString prefetch = mComicIdentifier + ':' + mNextIdentifierSuffix;
        mEngine->connectSource( prefetch, this );
        mEngine->query( prefetch );
    }
    if ( !mPreviousIdentifierSuffix.isEmpty() ) {
        const QString prefetch = mComicIdentifier + ':' + mPreviousIdentifierSuffix;
        mEngine->connectSource( prefetch, this );
        mEngine->query( prefetch );
    }

    updateView();
}

void ComicApplet::updateView()
{
    const bool tabsVisible = (mTabIdentifier.count() > 1);
    setTabBarVisible( tabsVisible );
    mLabelTop->setVisible( ( mShowComicAuthor || mShowComicTitle ) && !mLabelTop->text().isEmpty() );
    mLabelId->setVisible( mShowComicIdentifier && !mLabelId->text().isEmpty() );
    mLabelUrl->setVisible( mShowComicUrl && !mLabelUrl->text().isEmpty() );
    const int spacing = ( mLabelTop->isVisible() ? 2 : 0 );
    const int id = ( tabsVisible ? 1 : 0 );
    mCentralLayout->setItemSpacing( id, spacing );
    if ( mLabelId->isVisible() || mLabelUrl->isVisible() ) {
        mBottomLayout->setContentsMargins( 0, 2, 0, 0 );
    } else {
        mBottomLayout->setContentsMargins( 0, 0, 0, 0 );
    }

    updateButtons();
    updateContextMenu();

    Plasma::ToolTipContent toolTipData;
    if ( !mAdditionalText.isEmpty() ) {
        toolTipData = Plasma::ToolTipContent( mAdditionalText, QString() );
        toolTipData.setAutohide( false );
    }
    Plasma::ToolTipManager::self()->setContent( mMainWidget, toolTipData );

    if ( !mImage.isNull() ) {
        QTimer::singleShot( 1, this, SLOT(updateSize()) );//HACK
    }
}

void ComicApplet::createConfigurationInterface( KConfigDialog *parent )
{
    mConfigWidget = new ConfigWidget( dataEngine( "comic" ), mModel, mProxy, parent );
    mConfigWidget->setShowComicUrl( mShowComicUrl );
    mConfigWidget->setShowComicAuthor( mShowComicAuthor );
    mConfigWidget->setShowComicTitle( mShowComicTitle );
    mConfigWidget->setShowComicIdentifier( mShowComicIdentifier );
    mConfigWidget->setShowErrorPicture( mShowErrorPicture );
    mConfigWidget->setArrowsOnHover( mArrowsOnHover );
    mConfigWidget->setMiddleClick( mMiddleClick );
    mConfigWidget->setTabView( mTabView - 1);//-1 because counting starts at 0, yet we use flags that start at 1
    mConfigWidget->setCheckNewComicStripsIntervall( mCheckNewComicStripsIntervall );

    //not storing this value, since other applets might have changed it inbetween
    KConfigGroup global = globalConfig();
    const int maxComicLimit = global.readEntry( "maxComicLimit", CACHE_LIMIT );
    mConfigWidget->setMaxComicLimit( maxComicLimit );
    const int updateIntervall = global.readEntry( "updateIntervall", 3 );
    mConfigWidget->setUpdateIntervall( updateIntervall );

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage( mConfigWidget->comicSettings, i18n( "General" ), icon() );
    parent->addPage( mConfigWidget->appearanceSettings, i18n( "Appearance" ), "image" );
    parent->addPage( mConfigWidget->advancedSettings, i18n( "Advanced" ), "system-run" );

    connect( mConfigWidget, SIGNAL(maxSizeClicked()), this, SLOT(slotShowMaxSize()) );
    connect( parent, SIGNAL(applyClicked()), this, SLOT(applyConfig()) );
    connect( parent, SIGNAL(okClicked()), this, SLOT(applyConfig()) );
    connect(mConfigWidget, SIGNAL(enableApply()), parent, SLOT(settingsModified()));
}

void ComicApplet::applyConfig()
{
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();
    mShowComicIdentifier = mConfigWidget->showComicIdentifier();
    mShowErrorPicture = mConfigWidget->showErrorPicture();
    mArrowsOnHover = mConfigWidget->arrowsOnHover();
    mMiddleClick = mConfigWidget->middleClick();
    mTabView = mConfigWidget->tabView() + 1;//+1 because counting starts at 0, yet we use flags that start at 1
    mCheckNewComicStripsIntervall = mConfigWidget->checkNewComicStripsIntervall();

    //not storing this value, since other applets might have changed it inbetween
    KConfigGroup global = globalConfig();
    const int oldMaxComicLimit = global.readEntry( "maxComicLimit", CACHE_LIMIT );
    const int maxComicLimit = mConfigWidget->maxComicLimit();
    if ( oldMaxComicLimit != maxComicLimit ) {
        global.writeEntry( "maxComicLimit", maxComicLimit );
        mEngine->query( QLatin1String( "setting_maxComicLimit:" ) + QString::number( maxComicLimit ) );
    }


    globalComicUpdater->applyConfig( mConfigWidget );

    updateUsedComics();
    saveConfig();
    buttonBar();

    //make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    changeComic( mDifferentComic );
}

void ComicApplet::changeComic( bool differentComic )
{
    if ( differentComic ) {
        KConfigGroup cg = config();
        mLastIdentifierSuffix = cg.readEntry( "lastStripVisited_" + mComicIdentifier, QString() );
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
    KConfigGroup cg = config();
    int tab = 0;
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
            const QString identifier = data.data( Qt::UserRole ).toString();
            if ( mTabView & ShowText ) {
                name = data.data().toString();
            }
            if ( mTabView & ShowIcon ) {
                icon = data.data( Qt::DecorationRole ).value<QIcon>();
            }

            mTabBar->addTab( icon, name );

            //found a newer strip last time, which was not visited
            if ( mCheckNewComicStripsIntervall && !cg.readEntry( "lastStripVisited_" + identifier, true ) ) {
                mTabBar->setTabHighlighted( tab, true );
            }

            mTabIdentifier << identifier;
            ++tab;
        }
    }

    mActionNextNewStripTab->setVisible( mCheckNewComicStripsIntervall );
    mActionNextNewStripTab->setEnabled( mTabBar->hasHighlightedTabs() );

    delete mCheckNewStrips;
    if ( mCheckNewComicStripsIntervall ) {
        mCheckNewStrips = new CheckNewStrips( mTabIdentifier, mEngine, mCheckNewComicStripsIntervall, this );
        connect( mCheckNewStrips, SIGNAL(lastStrip(int,QString,QString)), this, SLOT(slotFoundLastStrip(int,QString,QString)) );
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

void ComicApplet::configChanged()
{
    KConfigGroup cg = config();
    mTabIdentifier = cg.readEntry( "tabIdentifier", QStringList( QString() ) );
    mComicIdentifier = mTabIdentifier.count() ? mTabIdentifier.at( 0 ) : QString();
    mShowComicUrl = cg.readEntry( "showComicUrl", false );
    mShowComicAuthor = cg.readEntry( "showComicAuthor", false );
    mShowComicTitle = cg.readEntry( "showComicTitle", false );
    mShowComicIdentifier = cg.readEntry( "showComicIdentifier", false );
    mShowErrorPicture = cg.readEntry( "showErrorPicture", true );
    mArrowsOnHover = cg.readEntry( "arrowsOnHover", true );
    mMiddleClick = cg.readEntry( "middleClick", true );
    mScaleComic = cg.readEntry( "scaleToContent_" + mComicIdentifier, false );
    mMaxStripNum[ mComicIdentifier ] = cg.readEntry( "maxStripNum_" + mComicIdentifier, 0 );
    mStoredIdentifierSuffix = cg.readEntry( "storedPosition_" + mComicIdentifier, QString() );
    mCheckNewComicStripsIntervall = cg.readEntry( "checkNewComicStripsIntervall", 30 );

    //use a decent default size
    const QSizeF tempMaxSize = isInPanel() ? QSizeF( 600, 250 ) : this->size();
    mMaxSize = cg.readEntry( "maxSize", tempMaxSize );
    mLastSize = mMaxSize;

    mTabView = cg.readEntry( "tabView", ShowText | ShowIcon );
    mSavingDir = cg.readEntry( "savingDir", QString() );

    globalComicUpdater->load();
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
    cg.writeEntry( "showComicUrl", mShowComicUrl );
    cg.writeEntry( "showComicAuthor", mShowComicAuthor );
    cg.writeEntry( "showComicTitle", mShowComicTitle );
    cg.writeEntry( "showComicIdentifier", mShowComicIdentifier );
    cg.writeEntry( "showErrorPicture", mShowErrorPicture );
    cg.writeEntry( "arrowsOnHover", mArrowsOnHover );
    cg.writeEntry( "middleClick", mMiddleClick );
    cg.writeEntry( "tabIdentifier", mTabIdentifier );
    cg.writeEntry( "tabView", mTabView );
    cg.writeEntry( "savingDir", mSavingDir );
    cg.writeEntry( "checkNewComicStripsIntervall", mCheckNewComicStripsIntervall );

    globalComicUpdater->save();
}

void ComicApplet::slotChosenDay( const QDate &date )
{
    if ( mComicType == Date ) {
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

void ComicApplet::slotFoundLastStrip( int index, const QString &identifier, const QString &suffix )
{
    KConfigGroup cg = config();
    if ( suffix != cg.readEntry( "lastStrip_" + identifier, QString() ) ) {
        kDebug() << identifier << "has a newer strip.";
        mTabBar->setTabHighlighted( index, true );
        cg.writeEntry( "lastStripVisited_" + identifier, false );
    }

    mActionNextNewStripTab->setEnabled( mTabBar->hasHighlightedTabs() );
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

void ComicApplet::slotGoJump()
{
    if ( mComicType == Number ) {
        QPointer<ChooseStripNumDialog> pageDialog = new ChooseStripNumDialog( 0, mCurrentIdentifierSuffix.toInt(), mFirstStripNum[ mComicIdentifier ], mMaxStripNum[ mComicIdentifier ] );
        if ( pageDialog->exec() == QDialog::Accepted ) {
            updateComic( QString::number( pageDialog->getStripNumber() ) );
        }
        delete pageDialog;
    } else if ( mComicType == Date ) {
        KDatePicker *calendar = new KDatePicker;
        calendar->setAttribute( Qt::WA_DeleteOnClose );//to have destroyed emitted upon closing
        calendar->setMinimumSize( calendar->sizeHint() );
        calendar->setDate( QDate::fromString( mCurrentIdentifierSuffix, "yyyy-MM-dd" ) );

        connect( calendar, SIGNAL(dateSelected(QDate)), this, SLOT(slotChosenDay(QDate)) );
        connect( calendar, SIGNAL(dateEntered(QDate)), this, SLOT(slotChosenDay(QDate)) );
        calendar->show();
    } else if ( mComicType == String ) {
        bool ok;
        const QString suffix = KInputDialog::getText( i18n( "Go to Strip" ), i18n( "Strip identifier:" ), mCurrentIdentifierSuffix, &ok );
        if ( ok ) {
            updateComic( suffix );
        }
    }
}

void ComicApplet::slotNextNewStrip()
{
    const int index = mTabBar->currentIndex();
    mTabBar->setCurrentIndex( mTabBar->nextHighlightedTab( index ) );
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

void ComicApplet::createComicBook()
{
    ComicArchiveDialog *dialog = new ComicArchiveDialog( mComicIdentifier, mComicTitle, mComicType, mCurrentIdentifierSuffix, mFirstIdentifierSuffix, getSavingDir() );
    dialog->setAttribute( Qt::WA_DeleteOnClose );//to have destroyed emitted upon closing
    connect( dialog, SIGNAL(archive(int,KUrl,QString,QString)), this, SLOT(slotArchive(int,KUrl,QString,QString)) );
    dialog->show();
}

void ComicApplet::slotArchive( int archiveType, const KUrl &dest, const QString &fromIdentifier, const QString &toIdentifier )
{
    mSavingDir = dest.directory();
    saveConfig();

    kDebug() << "Archiving:" << mComicIdentifier <<  archiveType << dest << fromIdentifier << toIdentifier;
    ComicArchiveJob *job = new ComicArchiveJob( dest, mEngine, static_cast< ComicArchiveJob::ArchiveType >( archiveType ), mComicType,  mComicIdentifier, this );
    job->setFromIdentifier( mComicIdentifier + ':' + fromIdentifier );
    job->setToIdentifier( mComicIdentifier + ':' + toIdentifier );
    if ( job->isValid() ) {
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotArchiveFinished(KJob*)) );
        KIO::getJobTracker()->registerJob( job );
        job->start();
    } else {
        kWarning() << "Archiving job is not valid.";
        delete job;
    }
}

void ComicApplet::slotArchiveFinished (KJob *job )
{
    if ( job->error() ) {
        KNotification::event( KNotification::Warning, i18n( "Archiving comic failed" ), job->errorText(), KIcon( "dialog-warning" ).pixmap( KIconLoader::SizeMedium ) );
    }
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

        if ( data[ "Error" ].toBool() ) {
            dataUpdated( QString(), data );
        }
    } else {
        kError() << "Either no identifier was specified or the engine could not be created:" << "id" << mComicIdentifier << "engine valid:" << ( mEngine && mEngine->isValid() );
        setConfigurationRequired( true );
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
}

QString ComicApplet::getSavingDir() const
{
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

    return dir;
}

void ComicApplet::slotSaveComicAs()
{
    KTemporaryFile tempFile;

    if ( !tempFile.open() ) {
        return;
    }

    // save image to temporary file
    mImage.save( tempFile.fileName(), "PNG" );

    KUrl srcUrl( tempFile.fileName() );

    QString dir = getSavingDir();
    const QString name = mComicTitle + " - " + mCurrentIdentifierSuffix + ".png";
    KUrl destUrl = KUrl( dir );
    destUrl.addPath( name );

    destUrl = KFileDialog::getSaveUrl( destUrl, "*.png" );
    if ( !destUrl.isValid() ) {
        return;
    }

    mSavingDir = destUrl.directory();
    saveConfig();

#ifdef HAVE_NEPOMUK
    bool worked = KIO::NetAccess::file_copy( srcUrl, destUrl );
    //store additional data using Nepomuk
    if (worked) {
        Nepomuk::Resource res(destUrl, NFO::FileDataObject());

        Nepomuk::Resource comicTopic("Comic", PIMO::Topic());
        comicTopic.setLabel(i18n("Comic"));

        if (!mAdditionalText.isEmpty() ) {
            res.setProperty(NIE::description(), mAdditionalText);
        }
        if ( ( mComicType == Date ) && !mShownIdentifierSuffix.isEmpty() ) {
            res.setProperty(NIE::contentCreated(), QDateTime::fromString(mShownIdentifierSuffix, Qt::ISODate));
        }
        if (!mComicTitle.isEmpty()) {
            Nepomuk::Resource topic(mComicTitle, PIMO::Topic());
            topic.setLabel(mComicTitle);
            topic.setProperty(PIMO::superTopic(), comicTopic);
            res.addTag(topic);
        } else {
//             res.addTag(comicTopic);//TODO activate this, see below
            ;
        }

        //FIXME also set the comic topic as tag, this is redundant, as topic has this as super topic
        //though at this point the gui does not manage to show the correct tags
        res.addTag(comicTopic);

        if (!mStripTitle.isEmpty()) {
            res.setProperty(NIE::title(), mStripTitle);
        }
        if (!mWebsiteUrl.isEmpty()) {
            Nepomuk::Resource copyEvent = Nepomuk::Utils::createCopyEvent(mImageUrl, destUrl, QDateTime(), mWebsiteUrl);
        }

        const QStringList authors = mComicAuthor.split(',', QString::SkipEmptyParts);
        foreach (QString author, authors) {
            author = author.trimmed();
            Nepomuk::Resource authorRes(author, NCO::PersonContact());
            authorRes.setProperty(NCO::fullname(), author);
            res.addProperty(NCO::creator(), authorRes);
        }
    }
#else
    KIO::NetAccess::file_copy( srcUrl, destUrl );
#endif
}

bool ComicApplet::eventFilter( QObject *receiver, QEvent *event )
{
    if ( receiver != mMainWidget ) {
        return Plasma::PopupApplet::eventFilter( receiver, event );
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
        case QEvent::GraphicsSceneMousePress:
            {
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>( event );
                if ( e->button() == Qt::LeftButton ) {
                    if ( mLabelUrl->isUnderMouse() ) {
                        if ( hasAuthorization( "LaunchApp" ) ) {
                            // link clicked
                            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
                            return true;
                        }
                    } else if ( mLabelId->isUnderMouse() ) {
                        // identifierSuffix clicked clicked
                        slotGoJump();
                        return true;
                    } else if ( mImageWidget->isUnderMouse() && ( mMainWidget->geometry().size() != mLastSize ) ) {
                        // only update the size by clicking on the image-rect if the user manual resized the applet
                        updateSize();
                        return true;
                    }
                } else if ( ( e->button() == Qt::MidButton ) && mMiddleClick ) { // handle full view
                    fullView();
                    return true;
                }
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
            connect( mPrevButton, SIGNAL(clicked()), this , SLOT(slotPreviousDay()) );
            l->addItem( mPrevButton );

            mZoomButton = new Plasma::PushButton( mFrame );
            mZoomButton->nativeWidget()->setIcon( KIcon( "zoom-original" ) );
            mZoomButton->nativeWidget()->setToolTip( i18n( "Show at actual size in a different view.  Alternatively, click with the middle mouse button on the comic." ) );
            mZoomButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mZoomButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mZoomButton, SIGNAL(clicked()), this, SLOT(fullView()) );
            l->addItem( mZoomButton );

            mNextButton = new Plasma::PushButton( mFrame );
            mNextButton->nativeWidget()->setIcon( KIcon( "arrow-right" ) );
            mNextButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mNextButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mNextButton, SIGNAL(clicked()), this , SLOT(slotNextDay()) );
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
        mFullViewWidget = new FullViewWidget;
    }

    if ( !mFullViewWidget->isVisible() ) {
        mFullViewWidget->setImage( mImage );
        mFullViewWidget->adaptPosition( mMainWidget->mapToScene( mMainWidget->pos() ).toPoint(), containment()->screen() );
        mFullViewWidget->show();
    }
}

#include "comic.moc"
