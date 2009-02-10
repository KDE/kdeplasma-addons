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
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>

#include <KConfigDialog>
#include <KDatePicker>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <knuminput.h>
#include <KPushButton>
#include <KRun>
#include <KTemporaryFile>

#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/PushButton>
#include <Plasma/ScrollBar>
#include <plasma/tooltipmanager.h>
#include <Plasma/Svg>

#include "configwidget.h"
#include "fullviewwidget.h"
#include "fadingitem.h"

static const int s_arrowWidth = 30;
static const int s_indentation = s_arrowWidth;
static const int s_scrollMargin = 2;

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
            numInput->setValue( current );

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
    : Plasma::Applet( parent, args ),
      mIdentifierSuffixNum( -1 ),
      mShowPreviousButton( false ),
      mShowNextButton( false ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false ),
      mFullViewWidget( 0 ),
      mEngine( 0 ),
      mFrame( 0 ),
      mFadingItem( 0 ),
      mPrevButton( 0 ),
      mNextButton( 0 ),
      mSvg( 0 ),
      mArrowsOnHover( false )
{
    setHasConfigurationInterface( true );
    resize( 480, 160 );
    setAspectRatioMode( Plasma::IgnoreAspectRatio );

    mScrollBarVert = new Plasma::ScrollBar( this );
    connect( mScrollBarVert, SIGNAL( valueChanged( int ) ), this, SLOT( slotScroll() ) );
    mScrollBarVert->setSingleStep( 80 );
    mScrollBarVert->hide();
    mScrollBarHoriz = new Plasma::ScrollBar( this );
    mScrollBarHoriz->setOrientation( Qt::Horizontal );
    connect( mScrollBarHoriz, SIGNAL( valueChanged( int ) ), this, SLOT( slotScroll() ) );
    mScrollBarHoriz->setSingleStep( 80 );
    mScrollBarHoriz->hide();
}

void ComicApplet::init()
{
    Plasma::ToolTipManager::self()->registerWidget( this );

    loadConfig();

    mSvg = new Plasma::Svg( this );
    mSvg->setImagePath( "widgets/arrows" );
    mSvg->setContainsMultipleImages( true );

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, SIGNAL( timeout() ), this, SLOT( checkDayChanged() ) );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes

    mActionGoFirst = new QAction( KIcon( "go-first" ), i18n( "&Jump to first Strip" ), this );
    mActions.append( mActionGoFirst );
    connect( mActionGoFirst, SIGNAL( triggered( bool ) ), this, SLOT( slotFirstDay() ) );

    mActionGoLast = new QAction( KIcon( "go-last" ), i18n( "&Jump to current Strip" ), this );
    mActions.append( mActionGoLast );
    connect( mActionGoLast, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentDay() ) );

    QAction *action = new QAction( KIcon( "document-save-as" ), i18n( "&Save Comic As..." ), this );
    mActions.append( action );
    connect( action, SIGNAL( triggered( bool ) ), this , SLOT( slotSaveComicAs() ) );

    mActionScaleContent = new QAction( KIcon( "zoom-original" ), i18nc( "@option:check Context menu of comic image", "&Actual Size" ), this );
    mActionScaleContent->setCheckable( true );
    mActionScaleContent->setChecked( mScaleComic );
    mActions.append( mActionScaleContent );
    connect( mActionScaleContent, SIGNAL( triggered( bool ) ), this , SLOT( slotScaleToContent() ) );

    connect( this, SIGNAL( geometryChanged() ), this, SLOT( slotSizeChanged() ) );

    mEngine = dataEngine( "comic" );
    connect( mEngine, SIGNAL( isBusy( bool ) ), this, SLOT( setBusy( bool ) ) );

    updateComic();
}

ComicApplet::~ComicApplet()
{
    delete mFullViewWidget;
}

void ComicApplet::dataUpdated( const QString&, const Plasma::DataEngine::Data &data )
{
    if ( data[ "Error" ].toBool() ) {
        if ( !data[ "Previous identifier suffix" ].toString().isEmpty() ) {
            updateComic( data[ "Previous identifier suffix" ].toString() );
        } else {
            setConfigurationRequired( true );
        }
        return;
    }

    mImage = data[ "Image" ].value<QImage>();
    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
    mFirstIdentifierSuffix = data[ "First strip identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mAdditionalText = data[ "Additional text" ].toString();
    mComicAuthor = data[ "Comic Author" ].toString();
    mComicTitle = data[ "Title" ].toString();
    mSuffixType = data[ "SuffixType" ].toString();

    QString temp = data[ "Identifier" ].toString();
    temp = temp.remove( mComicIdentifier + ':' );
    if ( mSuffixType == "Number" ) {
        mIdentifierSuffixNum = temp.toInt();
        if ( mMaxStripNum[ mComicIdentifier ] < mIdentifierSuffixNum ) {
            mMaxStripNum[ mComicIdentifier ] = mIdentifierSuffixNum;
        }

        temp = mFirstIdentifierSuffix.remove( mComicIdentifier + ':' );
        mFirstStripNum[ mComicIdentifier ] = temp.toInt();
    } else if ( mSuffixType == "Date" ) {
        mIdentifierSuffixDate = QDate::fromString( temp, "yyyy-MM-dd" );
    }

    // strip has been loaded, so its scaling-settings should be used
    mScaleComic = mActionScaleContent->isChecked();

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
    mConfigWidget = new ConfigWidget( dataEngine( "comic" ), parent );
    mConfigWidget->setComicIdentifier( mComicIdentifier );
    mConfigWidget->setShowComicUrl( mShowComicUrl );
    mConfigWidget->setShowComicAuthor( mShowComicAuthor );
    mConfigWidget->setShowComicTitle( mShowComicTitle );
    mConfigWidget->setShowComicIdentifier( mShowComicIdentifier );
    mConfigWidget->setArrowsOnHover( mArrowsOnHover );

    parent->addPage( mConfigWidget, i18n("General"), icon() );

    connect( parent, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
}

void ComicApplet::applyConfig()
{
    bool differentComic = ( mComicIdentifier != mConfigWidget->comicIdentifier() );
    bool checkButtonBar = ( mArrowsOnHover != mConfigWidget->arrowsOnHover() );
    mComicIdentifier = mConfigWidget->comicIdentifier();
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();
    mShowComicIdentifier = mConfigWidget->showComicIdentifier();
    mArrowsOnHover = mConfigWidget->arrowsOnHover();

    saveConfig();

    if ( differentComic ) {
        KConfigGroup cg = config();
        // assign mScaleComic the moment the new strip has been loaded (dataUpdated) as up to this point
        // the old one should be still shown with its scaling settings
        bool scaleComic = cg.readEntry( "scaleToContent_" + mComicIdentifier, false );
        mActionScaleContent->setChecked( scaleComic );

        updateComic();
    } else {
        updateSize();
    }
    if ( checkButtonBar ) {
        buttonBar();
        update();
    }
}

void ComicApplet::checkDayChanged()
{
    if ( ( mCurrentDay != QDate::currentDate() ) || mImage.isNull() )
        updateComic();

    mCurrentDay = QDate::currentDate();
}

void ComicApplet::loadConfig()
{
    KConfigGroup cg = config();
    mComicIdentifier = cg.readEntry( "comic", "" );
    mShowComicUrl = cg.readEntry( "showComicUrl", false );
    mShowComicAuthor = cg.readEntry( "showComicAuthor", false );
    mShowComicTitle = cg.readEntry( "showComicTitle", false );
    mShowComicIdentifier = cg.readEntry( "showComicIdentifier", false );
    mArrowsOnHover = cg.readEntry( "arrowsOnHover", true );
    mScaleComic = cg.readEntry( "scaleToContent_" + mComicIdentifier, false );
    mMaxSize = cg.readEntry( "maxSize", geometry().size() );
    mLastSize = mMaxSize;

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

void ComicApplet::slotSizeChanged()
{
    // if the applet was resized manually by the user
    if ( geometry().size() != mLastSize ) {
        mMaxSize = geometry().size();
        mScrollBarVert->setValue( 0 );
        mScrollBarHoriz->setValue( 0 );

        // if the current shown (!) -- not the one that
        // might be currently loaded -- strip is shown at
        // its original size, check if scrollbars are needed
        if ( mScaleComic ) {
            updateScrollBars();
        }

        KConfigGroup cg = config();
        cg.writeEntry( "maxSize", mMaxSize );
    }
}

void ComicApplet::updateScrollBars()
{
    const int scrollWidthSpace = mScrollBarVert->preferredSize().width() + s_scrollMargin;
    const int scrollHeightSpace = mScrollBarHoriz->preferredSize().height() + s_scrollMargin;

    bool hasScrollBarHoriz = mIdealSize.width() > mMaxSize.width();
    bool hasScrollBarVert = mIdealSize.height() + hasScrollBarHoriz * scrollHeightSpace  > mMaxSize.height();
    hasScrollBarHoriz = mIdealSize.width() + hasScrollBarVert * scrollWidthSpace > mMaxSize.width();

    mScrollBarVert->setVisible( hasScrollBarVert );
    mScrollBarHoriz->setVisible( hasScrollBarHoriz );
}

void ComicApplet::slotScroll()
{
    update( mImageRect );
}

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        const QRectF rect = contentsRect();
        const QPointF eventPos = event->pos();

        if ( mouseCursorInside( mRects[ PreviousButton ], eventPos ) ) {
            slotPreviousDay();
        } else if ( mouseCursorInside( mRects[ NextButton ], eventPos ) ) {
            slotNextDay();
        } else if ( mouseCursorInside( mRects[ WebsiteURL ], eventPos ) ) {
            // link clicked
            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
        } else if ( mouseCursorInside( mRects[ Identifier ], eventPos ) ) {
            // identifierSuffix clicked clicked
            if ( mSuffixType == "Number" ) {
                ChooseStripNumDialog pageDialog( 0, mIdentifierSuffixNum, mFirstStripNum[ mComicIdentifier ], mMaxStripNum[ mComicIdentifier ] );
                if ( pageDialog.exec() == QDialog::Accepted ) {
                    updateComic( QString::number( pageDialog.getStripNumber() ) );
                }
            } else if ( mSuffixType == "Date" ) {
                static KDatePicker *calendar = new KDatePicker();
                calendar->setMinimumSize( calendar->sizeHint() );
                calendar->setDate( mIdentifierSuffixDate );

                connect( calendar, SIGNAL( dateSelected( QDate ) ), this, SLOT( slotChosenDay( QDate ) ) );
                connect( calendar, SIGNAL( dateEntered( QDate ) ), this, SLOT( slotChosenDay( QDate ) ) );
                calendar->show();
            }
        } else if ( mouseCursorInside( mRects[ Image ], eventPos ) && ( geometry().size() != mLastSize ) ) {
            // only update the size by clicking on the image-rect if the user manual resized the applet
            updateSize();
        }
    } else if ( event->button() == Qt::MidButton ) { // handle full view
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
    if (mFullViewWidget) {
        mFullViewWidget->hide();
    }

    Applet::mouseReleaseEvent( event );
}

void ComicApplet::updateSize()
{
    if ( !mImage.isNull() && mImage.size().width() > 0 ) {
        int leftArea = ( mShowPreviousButton && !mArrowsOnHover ) ? s_arrowWidth : 0;
        int rightArea = ( mShowNextButton && !mArrowsOnHover ) ? s_arrowWidth : 0;
        int fmHeight = Plasma::Theme::defaultTheme()->fontMetrics().height();
        int topArea = ( ( ( mShowComicAuthor && mComicAuthor.isEmpty() ) ||
                          ( mShowComicTitle && ( !mStripTitle.isEmpty() || !mComicTitle.isEmpty() ) ) ) ? fmHeight : 0 );
        bool hasComicIdentifier = false;
        if ( ( ( mSuffixType == "Number" ) && ( mIdentifierSuffixNum != -1  ) ) ||
             ( ( mSuffixType == "Date" ) && mIdentifierSuffixDate.isValid() ) ) {
            hasComicIdentifier = true;
        }
        int bottomArea = ( ( mShowComicUrl && !mWebsiteUrl.isEmpty() ) || ( mShowComicIdentifier && hasComicIdentifier ) ? fmHeight : 0 );

        mIdealSize = QSizeF( geometry().size() - contentsRect().size() +
                 mImage.size() + QSizeF( leftArea + rightArea, topArea + bottomArea ) );

        qreal finalWidth = mMaxSize.width();
        qreal finalHeight = mMaxSize.height();

        // hide all scrollbars and check later if they are needed
        mScrollBarVert->hide();
        mScrollBarHoriz->hide();
        mScrollBarVert->setValue( 0 );
        mScrollBarHoriz->setValue( 0 );

        // uses the mIdealSize, as long as it is not larger, than the maximum size
        if ( mScaleComic ) {
            updateScrollBars();

            const int scrollWidthSpace = mScrollBarVert->preferredSize().width() + s_scrollMargin;
            const int scrollHeightSpace = mScrollBarHoriz->preferredSize().height() + s_scrollMargin;

            bool hasScrollBarHoriz = mScrollBarHoriz->isVisible();
            bool hasScrollBarVert = mScrollBarVert->isVisible();

            finalWidth = hasScrollBarHoriz ? mMaxSize.width() : mIdealSize.width() + hasScrollBarVert * scrollWidthSpace;
            finalHeight = hasScrollBarVert ? mMaxSize.height() : mIdealSize.height() + hasScrollBarHoriz * scrollHeightSpace;

            mLastSize = QSizeF( finalWidth, finalHeight );
        } else {
            int marginX = geometry().width() - contentsRect().width();
            int marginY = geometry().height() - contentsRect().height();
            int reservedWidth = leftArea + rightArea + marginX;
            int reservedHeight = topArea + bottomArea + marginY;
            qreal aspectRatio = qreal( mImage.size().height() ) / mImage.size().width();
            qreal imageHeight =  aspectRatio * ( mMaxSize.width() - reservedWidth );
            const QSizeF aspectSize = QSizeF( geometry().width(), imageHeight + topArea + bottomArea + marginY );


            // set height (width) for given width (height) keeping image aspect ratio
            if ( imageHeight <= mMaxSize.height() ) {
                finalHeight = imageHeight + reservedHeight;
            } else {
                qreal imageWidth = ( mMaxSize.height() - reservedHeight ) / aspectRatio;
                finalWidth = imageWidth + reservedWidth;
            }

            mLastSize = QSizeF( finalWidth, finalHeight );
        }

        resize( mLastSize );
    }
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect &contentRect )
{
    mRects.clear();

    // get the text at top
    QString tempTop;

    if ( mShowComicTitle ) {
        tempTop = mComicTitle;
        tempTop += ( ( !mStripTitle.isEmpty() && !mComicTitle.isEmpty() ) ? " - " : "" );
        tempTop += mStripTitle;
    }
    if ( mShowComicAuthor && !mComicAuthor.isEmpty() ) {
        tempTop = ( !tempTop.isEmpty() ? mComicAuthor + ": " + tempTop : mComicAuthor );
    }

    // create the text at top
    int topHeight = 0;
    if ( ( mShowComicAuthor || mShowComicTitle ) && !tempTop.isEmpty() ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        topHeight = fm.height();
        int height = contentRect.top();
        p->setPen( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
        mRects[ Top ] = QRectF( contentRect.left(), height, contentRect.width(), fm.height() );
        p->drawText( mRects[ Top ], Qt::AlignCenter, tempTop );
    }

    // get the correct identifier suffix
    if ( ( mSuffixType == "Number" ) && ( mIdentifierSuffixNum != -1  ) ) {
        mShownIdentifierSuffix = "# " + QString::number( mIdentifierSuffixNum );
    } else if ( ( mSuffixType == "Date" ) && mIdentifierSuffixDate.isValid() ) {
        mShownIdentifierSuffix = mIdentifierSuffixDate.toString( "yyyy-MM-dd" );
    } else {
        mShownIdentifierSuffix = QString();
    }

    // create the text at bottom
    int bottomHeight = 0;
    if ( ( !mWebsiteUrl.isEmpty() && mShowComicUrl ) ||
         ( !mShownIdentifierSuffix.isEmpty() && mShowComicIdentifier ) ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        bottomHeight += fm.height();
        int height = contentRect.bottom() - bottomHeight;
        p->setPen( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );

        if ( !mWebsiteUrl.isEmpty() && mShowComicUrl ) {
            mRects[ WebsiteURL ] = QRectF( contentRect.right() - s_indentation - fm.width( mWebsiteUrl.host() ), height,
                                         fm.width( mWebsiteUrl.host() ), fm.height() );
            p->drawText( mRects[ WebsiteURL ], Qt::AlignRight, mWebsiteUrl.host() );
        }

        if ( !mShownIdentifierSuffix.isEmpty() && mShowComicIdentifier ) {
            mRects[ Identifier ] = QRectF( contentRect.left() + s_indentation , height,
                                         fm.width( mShownIdentifierSuffix ), fm.height() );
            p->drawText( mRects[ Identifier ], Qt::AlignLeft, mShownIdentifierSuffix );
        }
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    p->setRenderHint( QPainter::SmoothPixmapTransform );

    const int scrollBarWidth = mScrollBarVert->preferredSize().width();
    const int scrollBarHeight = mScrollBarHoriz->preferredSize().height();
    int rightImageGap = mScrollBarVert->isVisible() ? scrollBarWidth + s_scrollMargin : 0;
    bottomHeight += mScrollBarHoriz->isVisible() ? scrollBarHeight + s_scrollMargin : 0;

    int leftImageGap = 0;
    int buttonMiddle = ( contentRect.height() / 2 ) + contentRect.top();
    if ( mShowPreviousButton && !mArrowsOnHover ) {
        mRects[ PreviousButton ] = QRectF( contentRect.left(), contentRect.top(), + s_arrowWidth, contentRect.height() );
        mSvg->paint( p, contentRect.left() - 5, buttonMiddle - 15, s_arrowWidth, 30, "left-arrow");

        leftImageGap += s_arrowWidth;
    }

    if ( mShowNextButton && !mArrowsOnHover ) {
        mRects[ NextButton ] = QRectF( contentRect.right() - s_arrowWidth, contentRect.top(), + s_arrowWidth, contentRect.height() );
        mSvg->paint( p, contentRect.right() - s_arrowWidth + 5, buttonMiddle - 15, s_arrowWidth, 30, "right-arrow");

        rightImageGap += s_arrowWidth;
    }

    mImageRect = QRect( contentRect.x() + leftImageGap,
                        contentRect.y() + topHeight,
                        contentRect.width() - leftImageGap - rightImageGap,
                        contentRect.height() - bottomHeight - topHeight );
    mRects[ Image ] = mImageRect;

    if ( mScrollBarVert->isVisible() ) {
        mRects[ ScrollBarVert ] = QRect( contentRect.right() - rightImageGap + s_scrollMargin,
                                  contentRect.top() + topHeight + s_scrollMargin,
                                  scrollBarWidth,
                                  contentRect.height() - topHeight - bottomHeight - 2 * s_scrollMargin );

        mScrollBarVert->setGeometry( mRects[ ScrollBarVert ] );
        mScrollBarVert->setRange( 0, mImage.height() - mImageRect.height() );
        mScrollBarVert->setPageStep( mImageRect.height() );
    }
    if ( mScrollBarHoriz->isVisible() ) {
        mRects[ ScrollBarHoriz ] = QRect( contentRect.left() + leftImageGap + s_scrollMargin,
                                   contentRect.bottom() - bottomHeight + s_scrollMargin,
                                   contentRect.width() - leftImageGap - rightImageGap - 2 * s_scrollMargin,
                                   scrollBarHeight );

        mScrollBarHoriz->setGeometry( mRects[ ScrollBarHoriz ] );
        mScrollBarHoriz->setRange( 0, mImage.width() - mImageRect.width() );
        mScrollBarHoriz->setPageStep( mImageRect.width() );
    }

    if ( mScaleComic ) {
        QRect shownImageRect = QRect( mScrollBarHoriz->value(), mScrollBarVert->value(), mImageRect.width(), mImageRect.height() );

        p->drawImage( mImageRect, mImage, shownImageRect );
    } else {
        p->drawImage( mImageRect, mImage );
    }

    // reposition of the hovering icons
    if ( mArrowsOnHover && ( geometry().size() == mLastSize ) ) {
        constraintsEvent( Plasma::SizeConstraint );
    }

    p->restore();
}

QList<QAction*> ComicApplet::contextualActions()
{
    return mActions;
}

void ComicApplet::wheelEvent( QGraphicsSceneWheelEvent *event )
{
    const QPointF eventPos = event->pos();
    const int numDegrees = event->delta() / 8;
    const int numSteps = numDegrees / 15;

    if ( mScrollBarVert->isVisible() && !mouseCursorInside( mRects[ ScrollBarHoriz ], eventPos ) ) {
        const int scroll = mScrollBarVert->singleStep();
        mScrollBarVert->setValue( mScrollBarVert->value() - numSteps * scroll );
    } else if ( mScrollBarHoriz->isVisible() ) {
        const int scroll = mScrollBarHoriz->singleStep();
        mScrollBarHoriz->setValue( mScrollBarHoriz->value() - numSteps * scroll );
    }
    QGraphicsItem::wheelEvent( event );
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    mEngine = dataEngine( "comic" );

    if ( !mComicIdentifier.isEmpty() && mEngine && mEngine->isValid() ) {
        const QString identifier = mComicIdentifier + ':' + identifierSuffix;
        mEngine->disconnectSource( identifier, this );
        mEngine->connectSource( identifier, this );
        const Plasma::DataEngine::Data data = mEngine->query( identifier );

        if ( data[ "Error" ].toBool() && data[ "Previous identifier suffix" ].toString().isEmpty() ) {
            setConfigurationRequired( true );
        }
    } else {
        setConfigurationRequired( true );
    }
}

void ComicApplet::updateButtons()
{
    if ( mNextIdentifierSuffix.isEmpty() )
        mShowNextButton = false;
    else
        mShowNextButton = true;

    if ( mPreviousIdentifierSuffix.isEmpty() )
        mShowPreviousButton = false;
    else
        mShowPreviousButton = true;

    if ( mNextButton && mPrevButton ) {
        mNextButton->setEnabled( mShowNextButton );
        mPrevButton->setEnabled( mShowPreviousButton );
    }
}

void ComicApplet::updateContextMenu()
{
    mActionGoFirst->setVisible( !mFirstIdentifierSuffix.isEmpty() );
    mActionGoFirst->setEnabled( !mPreviousIdentifierSuffix.isEmpty() );
    mActionGoLast->setEnabled( !mNextIdentifierSuffix.isEmpty() );
}

void ComicApplet::slotSaveComicAs()
{
    KTemporaryFile tempFile;

    if ( !tempFile.open() )
        return;

    // save image to temporary file
    mImage.save( tempFile.fileName(), "PNG" );

    KUrl srcUrl( tempFile.fileName() );

    KUrl destUrl = KFileDialog::getSaveUrl( KUrl(), "*.png" );
    if ( !destUrl.isValid() )
        return;

    KIO::NetAccess::file_copy( srcUrl, destUrl );
}

void ComicApplet::constraintsEvent( Plasma::Constraints constraints )
{
    if ( constraints && Plasma::SizeConstraint && mFrame ) {
        qreal bottom = mImageRect.bottom();
        QPointF buttons( ( size().width() - mFrame->size().width() ) / 2,
                         bottom - mFrame->size().height() - 5 );
        if ( buttons.y() < 0 || buttons.y() > size().height() ) {
            buttons.setY( contentsRect().bottom() - mFrame->size().height() - 5 );
        }
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

void ComicApplet::hoverMoveEvent( QGraphicsSceneHoverEvent *event  )
{
    QRectF rect = contentsRect();
    const QPointF eventPos = event->pos();
    QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();

    if ( mouseCursorInside( mRects[ WebsiteURL ], eventPos ) ) {
        setCursor( Qt::PointingHandCursor );
    } else if ( mouseCursorInside( mRects[ Identifier ], eventPos ) ) {
        setCursor( Qt::PointingHandCursor );
    } else if ( mouseCursorInside( mRects[ PreviousButton ], eventPos ) ) {
        setCursor( Qt::PointingHandCursor );
    } else if ( mouseCursorInside( mRects[ NextButton ], eventPos ) ) {
        setCursor( Qt::PointingHandCursor );
    } else if ( hasCursor() ) {
        unsetCursor();
    }
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

    KConfigGroup cg = config();
    cg.writeEntry( "scaleToContent_" + mComicIdentifier,  mScaleComic );

    updateSize();
}

bool ComicApplet::mouseCursorInside( const QRectF &rect, const QPointF &position )
{
    if ( !position.isNull() && rect.isValid() &&
         position.y() > rect.top() &&
         position.y() < rect.bottom() &&
         position.x() > rect.left() &&
         position.x() < rect.right() ) {
        return true;
    } else {
        return false;
    }
}

void ComicApplet::buttonBar()
{
    if ( mArrowsOnHover ) {
        if ( !mFrame ) {
            mFrame = new Plasma::Frame( this );
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
            updateButtons();
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
