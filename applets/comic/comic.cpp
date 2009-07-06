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

#include "comic.h"

#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <KConfigDialog>
#include <KDatePicker>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <knuminput.h>
#include <KPushButton>
#include <KRun>
#include <KTemporaryFile>

#include "config-nepomuk.h"
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
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/TabBar>
#include <plasma/tooltipmanager.h>

#include "arrowwidget.h"
#include "configwidget.h"
#include "fullviewwidget.h"
#include "fadingitem.h"
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

static const int s_tabHeight = 24;//used because there is no initial preferredHeight set

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mShowPreviousButton( false ),
      mShowNextButton( false ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false ),
      mArrowsOnHover( true ),
      mMiddleClick( true ),
      mFullViewWidget( 0 ),
      mActionShop( 0 ),
      mEngine( 0 ),
      mFrame( 0 ),
      mFadingItem( 0 ),
      mPrevButton( 0 ),
      mNextButton( 0 )
{
    setHasConfigurationInterface( true );
    setMinimumSize( QSizeF( 80, 40 ) );
    resize( 600, 250 );
    setAspectRatioMode( Plasma::IgnoreAspectRatio );

    mTabBar = new Plasma::TabBar( this );
    mTabBar->hide();
    connect( mTabBar, SIGNAL( currentChanged( int ) ), this, SLOT( slotTabChanged( int ) ) );

    mLabelTop = new Plasma::Label( this );
    mLabelTop->setMinimumWidth( 0 );
    mLabelTop->nativeWidget()->setWordWrap( false );
    mLabelTop->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mLabelTop->setAlignment( Qt::AlignCenter );
    mLabelTop->hide();

    mImageWidget = new ImageWidget( this );
    mImageWidget->setZValue( 0 );
    mImageWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    mImageWidget->hide();

    mLabelId = new Plasma::Label( this );
    mLabelId->setMinimumWidth( 0 );
    mLabelId->nativeWidget()->setWordWrap( false );
    mLabelId->nativeWidget()->setCursor( Qt::PointingHandCursor );
    mLabelId->nativeWidget()->setToolTip( i18n( "Jump to Strip ..." ) );
    mLabelId->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mLabelId->hide();

    mLabelUrl = new Plasma::Label( this );
    mLabelUrl->setMinimumWidth( 0 );
    mLabelUrl->nativeWidget()->setWordWrap( false );
    if ( hasAuthorization( "LaunchApp" ) ) {
        mLabelUrl->nativeWidget()->setCursor( Qt::PointingHandCursor );
        mLabelUrl->nativeWidget()->setToolTip( i18n( "Visit the comic website" ) );
    }
    mLabelUrl->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mLabelUrl->hide();

    mLeftArrow = new ArrowWidget( this );
    mLeftArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    mLeftArrow->setCursor( Qt::PointingHandCursor );
    mLeftArrow->hide();
    mLeftArrow->setGeometry( QRectF( QPointF( 0, 0 ), mLeftArrow->preferredSize() ) );//to intially have a size -- needed in updateSize
    connect( mLeftArrow, SIGNAL( clicked() ), this, SLOT( slotPreviousDay() ) );

    mRightArrow = new ArrowWidget( this );
    mRightArrow->setDirection( Plasma::Right );
    mRightArrow->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    mRightArrow->setCursor( Qt::PointingHandCursor );
    mRightArrow->hide();
    mRightArrow->setGeometry( QRectF( QPointF( 0, 0 ), mRightArrow->preferredSize() ) );//to intially have a size -- needed in updateSize
    connect( mRightArrow, SIGNAL( clicked() ), this, SLOT( slotNextDay() ) );

#ifdef HAVE_NEPOMUK
    //for manually saving the comics
    Nepomuk::ResourceManager::instance()->init();
#endif
}

void ComicApplet::createLayout()
{
    QGraphicsLinearLayout *newLayout = new QGraphicsLinearLayout;
    newLayout->setContentsMargins( 0, 0, 0, 0 );
    newLayout->setSpacing( 1 );

    QGraphicsLinearLayout *centralLayout = new QGraphicsLinearLayout;
    centralLayout->setOrientation( Qt::Vertical );
    centralLayout->setContentsMargins( 0, 0, 0, 0 );
    centralLayout->setSpacing( 0 );

    QGraphicsLinearLayout *bottomLayout = 0;

    mLeftArrow->setVisible( mShowPreviousButton && !mArrowsOnHover );
    if ( mLeftArrow->isVisible() ) {
        newLayout->addItem( mLeftArrow );
    }

    mTabBar->setVisible( mShowTabBar && mUseTabs );
    if ( mTabBar->isVisible() ) {
        centralLayout->addItem( mTabBar );
        centralLayout->setItemSpacing( centralLayout->count() - 1, 5 );
    }

    mLabelTop->setVisible( ( mShowComicAuthor || mShowComicTitle ) && !mLabelTop->text().isEmpty() );
    if ( mLabelTop->isVisible() ) {
        centralLayout->addItem( mLabelTop);
    }

    mImageWidget->show();
    centralLayout->addItem( mImageWidget );

    mLabelId->setVisible( mShowComicIdentifier && !mLabelId->text().isEmpty() );
    mLabelUrl->setVisible( mShowComicUrl && !mLabelUrl->text().isEmpty() );
    if ( mLabelId->isVisible() || mLabelUrl->isVisible() ) {
        bottomLayout = new QGraphicsLinearLayout;
        bottomLayout->setContentsMargins( 0, 0, 0, 0 );
        bottomLayout->setSpacing( 0 );

        if ( mLabelId->isVisible() ) {
            bottomLayout->addItem( mLabelId );
        }
        bottomLayout->addStretch( 1 );
        if ( mLabelUrl->isVisible() ) {
            bottomLayout->addItem( mLabelUrl );
        }
        centralLayout->addItem( bottomLayout );
    }
    newLayout->addItem( centralLayout );

    mRightArrow->setVisible( mShowNextButton && !mArrowsOnHover );
    if ( mRightArrow->isVisible() ) {
        newLayout->addItem( mRightArrow );
    }

    setLayout( newLayout );
}

void ComicApplet::init()
{
    Plasma::ToolTipManager::self()->registerWidget( this );

    loadConfig();

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

    connect( this, SIGNAL( geometryChanged() ), this, SLOT( slotSizeChanged() ) );

    updateTabBar();
    changeComic( true );
}

ComicApplet::~ComicApplet()
{
    delete mFullViewWidget;
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
        mShownIdentifierSuffix = i18nc("an abbrevation for Number", "# %1", mCurrentIdentifierSuffix);
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

    updateButtons();
    updateContextMenu();

    Plasma::ToolTipContent toolTipData;
    if ( !mAdditionalText.isEmpty() ) {
        toolTipData = Plasma::ToolTipContent( mAdditionalText, QString() );
        toolTipData.setAutohide( false );
    }
    Plasma::ToolTipManager::self()->setContent( this, toolTipData );

    if ( !mImage.isNull() ) {
        updateSize();
        update();
    }
}

void ComicApplet::createConfigurationInterface( KConfigDialog *parent )
{
    //to not have tab switches while configurating things
    mReloadTimer->stop();

    mConfigWidget = new ConfigWidget( dataEngine( "comic" ), parent );
    mConfigWidget->setComicIdentifier( mTabIdentifier );
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
    bool checkButtonBar = ( mArrowsOnHover != mConfigWidget->arrowsOnHover() );
    bool differentComic = ( mComicIdentifier != mConfigWidget->comicIdentifier().at( 0 ) );

    mTabIdentifier = mConfigWidget->comicIdentifier();
    mComicIdentifier = mTabIdentifier.at( 0 );
    mTabText = mConfigWidget->comicName();
    mComicTitle = mTabText.at( 0 );
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();
    mShowComicIdentifier = mConfigWidget->showComicIdentifier();
    mArrowsOnHover = mConfigWidget->arrowsOnHover();
    mMiddleClick = mConfigWidget->middleClick();
    QTime time = mConfigWidget->tabSwitchTime();
    mSwitchTabTime = time.second() + time.minute() * 60 + time.hour() * 3600;
    mShowTabBar = !mConfigWidget->hideTabBar();
    mUseTabs = mConfigWidget->useTabs();
    mSwitchTabs = mConfigWidget->switchTabs();

    slotStartTimer();
    updateTabBar();
    saveConfig();

    changeComic( differentComic );

    if ( checkButtonBar ) {
        buttonBar();
        update();
    }
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

void ComicApplet::updateTabBar()
{
    for ( int i = mTabBar->count(); i > 0; --i ) {
        mTabBar->removeTab( i - 1 );
    }
    for ( int i = 0; i < mTabIdentifier.count(); ++i ) {
        mTabBar->addTab( mTabText.at( i ) );
    }
}

void ComicApplet::slotTabChanged( int newIndex )
{
    if (newIndex >= mTabIdentifier.count()) {
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
    mStoredIdentifierSuffix = cg.readEntry( "storedPosition_" + mComicIdentifier, "" );
    mMaxSize = cg.readEntry( "maxSize", geometry().size() );
    mLastSize = mMaxSize;
    mSwitchTabTime = cg.readEntry( "switchTabTime", 10 );// 10 seconds as default
    mShowTabBar = cg.readEntry( "showTabBar", true );
    mTabText = cg.readEntry( "tabText", QStringList( QString() ) );
    mComicTitle = mTabText.count() ? mTabText.at( 0 ) : QString();
    mUseTabs = cg.readEntry( "useTabs", false );
    mSwitchTabs = cg.readEntry( "switchTabs", false );

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
    cg.writeEntry( "tabText", mTabText );
    cg.writeEntry( "useTabs", mUseTabs );
    cg.writeEntry( "switchTabs", mSwitchTabs );
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
    if ( mSwitchTabTime && mSwitchTabs ) {
        mReloadTimer->start( mSwitchTabTime * 1000 );
    } else {
        mReloadTimer->stop();
    }
}

void ComicApplet::slotGoJump()
{
    if ( mSuffixType == "Number" ) {
        ChooseStripNumDialog pageDialog( 0, mCurrentIdentifierSuffix.toInt(), mFirstStripNum[ mComicIdentifier ], mMaxStripNum[ mComicIdentifier ] );
        if ( pageDialog.exec() == QDialog::Accepted ) {
            updateComic( QString::number( pageDialog.getStripNumber() ) );
        }
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
    if ( geometry().size() != mLastSize ) {
        mMaxSize = geometry().size();

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

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    slotStartTimer();
    if ( event->button() == Qt::LeftButton ) {
        if ( mLabelUrl->isUnderMouse() ) {
            if ( hasAuthorization( "LaunchApp" ) ) {
                // link clicked
                KRun::runUrl( mWebsiteUrl, "text/html", 0 );
            }
        } else if ( mLabelId->isUnderMouse() ) {
            // identifierSuffix clicked clicked
            slotGoJump();
        } else if ( mImageWidget->isUnderMouse() && ( geometry().size() != mLastSize ) ) {
            // only update the size by clicking on the image-rect if the user manual resized the applet
            updateSize();
        }
    } else if ( ( event->button() == Qt::MidButton ) && mMiddleClick ) { // handle full view
        if ( !mFullViewWidget ) {
            mFullViewWidget = new FullViewWidget();
        }

        if ( !mFullViewWidget->isVisible() ) {
            mFullViewWidget->setImage( mImage );
            mFullViewWidget->adaptPosition( mapToScene( 0, 0 ).toPoint() );
            mFullViewWidget->show();
        }
    }

    event->ignore();
}

void ComicApplet::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    if ( mFullViewWidget ) {
        mFullViewWidget->hide();
    }

    Applet::mouseReleaseEvent( event );
}

void ComicApplet::wheelEvent( QGraphicsSceneWheelEvent *event ) {
    slotStartTimer();
    if ( mImageWidget->isUnderMouse() && ( event->modifiers() == Qt::ControlModifier ) ) {
        const QPointF eventPos = event->pos();
        const int numDegrees = event->delta() / 8;
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
        event->accept();
    }
    Applet::wheelEvent( event );
}


void ComicApplet::updateSize()
{
    //HACK to work around shortcomings in the layout code FIXME later
    mLabelId->setPreferredWidth( mLabelId->nativeWidget()->sizeHint().width() );
    mLabelUrl->setPreferredWidth( mLabelUrl->nativeWidget()->sizeHint().width() );

    //leftArea and rightArea are not completly correct when their size is smaller then the preferredSize
    int leftArea = ( mShowPreviousButton && !mArrowsOnHover ) ? mLeftArrow->preferredSize().width() + 1 : 0;
    int rightArea = ( mShowNextButton && !mArrowsOnHover ) ? mRightArrow->preferredSize().width() + 1 : 0;
    int topArea = mTabBar->nativeWidget()->height() ? mTabBar->nativeWidget()->height() : s_tabHeight;
    topArea = ( mShowTabBar && mUseTabs ) ? topArea + 5 : 0;
    topArea += ( ( mShowComicAuthor || mShowComicTitle ) && !mLabelTop->text().isEmpty() ) ? mLabelTop->nativeWidget()->height() : 0;
    int bottomArea = ( mShowComicUrl && !mLabelUrl->text().isEmpty() ) ? mLabelUrl->nativeWidget()->height() : 0;
    bottomArea = ( mShowComicIdentifier && !mLabelId->text().isEmpty() ) ? mLabelId->nativeWidget()->height() : bottomArea;

    QSizeF margins = geometry().size() - contentsRect().size();
    QSizeF availableSize = mMaxSize - margins;
    availableSize.setHeight( availableSize.height() - topArea - bottomArea );
    availableSize.setWidth( availableSize.width() - leftArea - rightArea );
    mImageWidget->setAvailableSize( availableSize );

    mLastSize = mImageWidget->preferredSize() + margins;
    mLastSize.setHeight( mLastSize.height() + topArea + bottomArea );
    mLastSize.setWidth( mLastSize.width() + leftArea + rightArea );

    createLayout();
    resize( mLastSize );
    emit sizeHintChanged(Qt::PreferredSize);
    emit appletTransformedItself();
}

QSizeF ComicApplet::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which != Qt::PreferredSize) {
        return Applet::sizeHint(which, constraint);
    } else {
        return mLastSize;
    }
}

QList<QAction*> ComicApplet::contextualActions()
{
    return mActions;
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
        static QString oldSource = identifier;

        //disconnecting of the oldSource is needed, otherwise you could get data for comics you are not looking at if you use tabs
        //if there was an error only disconnect the oldSource if it had nothing to do with the error or if the comic changed, that way updates of the error can come in
        if ( !mIdentifierError.isEmpty() && !mIdentifierError.contains(mComicIdentifier) ) {
            mEngine->disconnectSource( mIdentifierError, this );
            mIdentifierError.clear();
        }
        if ( ( mIdentifierError != oldSource ) && ( oldSource != identifier ) ) {
            mEngine->disconnectSource( oldSource, this );
        }
        oldSource = identifier;

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

    KUrl destUrl = KFileDialog::getSaveUrl( KUrl(), "*.png" );
    if ( !destUrl.isValid() ) {
        slotStartTimer();
        return;
    }

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

void ComicApplet::constraintsEvent( Plasma::Constraints constraints )
{
    if ( constraints && Plasma::SizeConstraint && mFrame ) {
        QPointF buttons( ( size().width() - mFrame->size().width() ) / 2,
                         contentsRect().bottom() - mFrame->size().height() - 5 );
        mFrame->setPos( buttons );
    }
}

void ComicApplet::hoverEnterEvent( QGraphicsSceneHoverEvent *event )
{
    if ( !configurationRequired() && mFadingItem && !mFadingItem->isVisible() ) {
        mFadingItem->showItem();
    }

    Applet::hoverEnterEvent( event );
}

void ComicApplet::hoverLeaveEvent( QGraphicsSceneHoverEvent *event )
{
    if ( mFadingItem && mFadingItem->isVisible() ) {
        mFadingItem->hideItem();
    }

    Applet::hoverLeaveEvent( event );
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
            mFrame = new Plasma::Frame( this );
            mFrame->setZValue( 10 );
            QGraphicsLinearLayout *l = new QGraphicsLinearLayout();
            mPrevButton = new Plasma::PushButton( mFrame );
            mPrevButton->nativeWidget()->setIcon( KIcon( "arrow-left" ) );
            mPrevButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            mPrevButton->setMaximumSize( IconSize( KIconLoader::MainToolbar ), IconSize( KIconLoader::MainToolbar ) );
            connect( mPrevButton, SIGNAL( clicked() ), this , SLOT( slotPreviousDay() ) );
            l->addItem( mPrevButton );
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
            mFrame->hide();
            mFadingItem = new FadingItem( mFrame );
            mFadingItem->hide();
            // Set frame position
            constraintsEvent( Plasma::SizeConstraint );
        }
    } else {
        delete mFrame;
        mFrame = 0;
        mPrevButton = 0;
        mNextButton = 0;
        mFadingItem = 0;
    }
}

#include "comic.moc"
