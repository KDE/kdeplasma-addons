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
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>

#include <KConfigDialog>
#include <KDatePicker>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <knuminput.h>
#include <KRun>
#include <KTemporaryFile>

#include <Plasma/Theme>
#include <plasma/tooltipmanager.h>

#include "configwidget.h"
#include "fullviewwidget.h"
#include "pluginmanager.h"

static const int s_arrowWidth = 30;

//NOTE based on GotoPageDialog KDE/kdegraphics/okular/part.cpp
//BEGIN choose a strip dialog
class ChooseStripNumDialog : public KDialog
{
    public:
        ChooseStripNumDialog( QWidget *parent, int current, int max ) : KDialog( parent )
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
            numInput->setRange( 1, max );
            numInput->setEditFocus( true );
            numInput->setSliderEnabled( true );
            numInput->setValue( current );

            QLabel *label = new QLabel( i18n( "&Strip Number:" ), widget );
            label->setBuddy( numInput );
            topLayout->addWidget( label );
            topLayout->addWidget( numInput) ;
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
      mShowPreviousButton( true ),
      mShowNextButton( false ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false )
{
    setHasConfigurationInterface( true );
    resize( 480, 160 );
    setAspectRatioMode( Plasma::KeepAspectRatio );
}

void ComicApplet::init()
{
    Plasma::ToolTipManager::self()->registerWidget( this );

    loadConfig();

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, SIGNAL( timeout() ), this, SLOT( checkDayChanged() ) );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes

    updateButtons();

    mActionGoFirst = new QAction( KIcon( "go-first" ), i18n( "&Jump to first Strip" ), this );
    mActions.append( mActionGoFirst );
    connect( mActionGoFirst, SIGNAL( triggered( bool ) ), this, SLOT( slotFirstDay() ) );

    mActionGoLast = new QAction( KIcon( "go-last" ), i18n( "&Jump to current Strip" ), this );
    mActions.append( mActionGoLast );
    connect( mActionGoLast, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentDay() ) );

    QAction *action = new QAction( KIcon( "document-save-as" ), i18n( "&Save Comic As..." ), this );
    mActions.append( action );
    connect( action, SIGNAL( triggered( bool ) ), this , SLOT( slotSaveComicAs() ) );

    mFullViewWidget = new FullViewWidget();
    mFullViewWidget->hide();

    Solid::Networking::Status status = Solid::Networking::status();
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown )
        updateComic();

    connect( Solid::Networking::notifier(), SIGNAL( statusChanged( Solid::Networking::Status ) ),
             this, SLOT( networkStatusChanged( Solid::Networking::Status ) ) );
}

ComicApplet::~ComicApplet()
{
    delete mFullViewWidget;
}

void ComicApplet::dataUpdated( const QString&, const Plasma::DataEngine::Data &data )
{
    setCursor( Qt::ArrowCursor );

    mImage = data[ "Image" ].value<QImage>();
    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
    mFirstDayIdentifierSuffix = data[ "First strip identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mAdditionalText = data[ "Additional text" ].toString();
    mComicAuthor = data[ "Comic Author" ].toString();
    mComicTitle = PluginManager::Instance()->comicTitle( mComicIdentifier );
    mSuffixType = PluginManager::Instance()->suffixType( mComicIdentifier );

    QString temp = data[ "Identifier" ].toString();
    int index = temp.indexOf( ':' );
    temp = temp.mid( index + 1 );
    if ( mSuffixType == "Number" ) {
        mIdentifierSuffixNum = temp.toInt();
        if ( mMaxStripNum[ mComicIdentifier ] < mIdentifierSuffixNum ) {
            mMaxStripNum[ mComicIdentifier ] = mIdentifierSuffixNum;
        }
    } else if ( mSuffixType == "Date" ) {
        mIdentifierSuffixDate = QDate::fromString( temp, "yyyy-MM-dd" );
    }

    updateButtons();
    updateContextMenu();

    if ( !mImage.isNull() ) {
        updateSize();
        prepareGeometryChange();
        updateGeometry();
        update();
    }
}

void ComicApplet::createConfigurationInterface( KConfigDialog *parent )
{
    mConfigWidget = new ConfigWidget( parent );
    mConfigWidget->setComicIdentifier( mComicIdentifier );
    mConfigWidget->setShowComicUrl( mShowComicUrl );
    mConfigWidget->setShowComicAuthor( mShowComicAuthor );
    mConfigWidget->setShowComicTitle( mShowComicTitle );
    mConfigWidget->setShowComicIdentifier( mShowComicIdentifier );

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage( mConfigWidget, parent->windowTitle(), icon() );

    connect( parent, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
}

void ComicApplet::applyConfig()
{
    bool differentComic = ( mComicIdentifier != mConfigWidget->comicIdentifier() );
    mComicIdentifier = mConfigWidget->comicIdentifier();
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();
    mShowComicIdentifier = mConfigWidget->showComicIdentifier();

    saveConfig();

    if ( differentComic ) {
        updateComic();
    }
}

void ComicApplet::networkStatusChanged( Solid::Networking::Status status )
{
    if ( status == Solid::Networking::Connected )
        updateComic();
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
    mComicIdentifier = cg.readEntry( "comic", "garfield" );
    mShowComicUrl = cg.readEntry( "showComicUrl", false );
    mShowComicAuthor = cg.readEntry( "showComicAuthor", false );
    mShowComicTitle = cg.readEntry( "showComicTitle", false );
    mShowComicIdentifier = cg.readEntry( "showComicIdentifier", false );
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
    cg.writeEntry( "showComicUrl", mShowComicUrl );
    cg.writeEntry( "showComicAuthor", mShowComicAuthor );
    cg.writeEntry( "showComicTitle", mShowComicTitle );
    cg.writeEntry( "showComicIdentifier", mShowComicIdentifier );
}

void ComicApplet::slotChosenDay( const QDate &date )
{
    if ( mSuffixType == "Date" ) {
        if ( date <= mCurrentDay ) {
            QDate temp = QDate::fromString( mFirstDayIdentifierSuffix, "yyyy-MM-dd" );
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
    updateComic( mFirstDayIdentifierSuffix );
}

void ComicApplet::slotCurrentDay()
{
    updateComic( QString() );
}

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();

        const QRectF rect = contentsRect();
        if ( mShowPreviousButton && event->pos().x() > rect.left() &&
                                    event->pos().x() < ( rect.left() + s_arrowWidth ) ) {
            slotPreviousDay();
        } else if ( mShowNextButton && event->pos().x() > ( rect.right() - s_arrowWidth ) &&
                                       event->pos().x() < rect.right() ) {
            slotNextDay();
        } else if ( !mWebsiteUrl.isEmpty() && mShowComicUrl &&
                    event->pos().y() > ( rect.bottom() - fm.height() ) &&
                    event->pos().x() > ( rect.right() - fm.width( mWebsiteUrl.host() ) - s_arrowWidth) ) {
            // link clicked
            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
        } else if ( !mShownIdentifierSuffix.isEmpty() && mShowComicIdentifier &&
                    event->pos().y() > ( rect.bottom() - fm.height() ) &&
                    event->pos().x() > ( rect.left() + s_arrowWidth ) &&
                    event->pos().x() < ( rect.left() + s_arrowWidth + fm.width( mShownIdentifierSuffix ) ) ) {
            // identifierSuffix clicked clicked
            if ( mSuffixType == "Number" ) {
                ChooseStripNumDialog pageDialog( 0, mIdentifierSuffixNum, mMaxStripNum[ mComicIdentifier ] );
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
        }
    } else if ( event->button() == Qt::MidButton ) { // handle full view
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
    mFullViewWidget->hide();

    Applet::mouseReleaseEvent( event );
}

void ComicApplet::updateSize()
{
    if ( !mImage.isNull() && mImage.size().width() > 0 ) {
        // Set height for given width keeping image aspect ratio
        const QSize size = mImage.size();
        int leftArea = mShowPreviousButton ? s_arrowWidth : 0;
        int rightArea = mShowNextButton ? s_arrowWidth : 0;
        qreal aspectRatio = qreal( size.height() ) / size.width();
        qreal imageHeight = aspectRatio * ( geometry().width() - leftArea - rightArea );
        int fmHeight = Plasma::Theme::defaultTheme()->fontMetrics().height();
        int topArea = ( ( mShowComicAuthor || mShowComicTitle ) ? fmHeight : 0 );
        int bottomArea = ( mShowComicUrl || mShowComicIdentifier ? fmHeight : 0 );
        resize( geometry().width(), imageHeight + topArea + bottomArea );
    }
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect &contentRect )
{
    // get the text at top
    Plasma::ToolTipManager::ToolTipContent toolTipData;
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
        p->drawText( QRectF( contentRect.left(), height, contentRect.width(), fm.height() ),
                     Qt::AlignCenter, tempTop );
    }

    // get the correct identifier suffix
    if ( ( mSuffixType == "Number" ) && ( mIdentifierSuffixNum != -1  ) ) {
        mShownIdentifierSuffix = "# " + QString::number( mIdentifierSuffixNum );
    } else if ( ( mSuffixType == "Date" ) && mIdentifierSuffixDate.isValid() ) {
        mShownIdentifierSuffix = mIdentifierSuffixDate.toString( "yyyy-MM-dd" );
    }

    // create the text at bottom
    int urlHeight = 0;
    if ( ( !mWebsiteUrl.isEmpty() && mShowComicUrl ) ||
         ( !mShownIdentifierSuffix.isEmpty() && mShowComicIdentifier ) ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        urlHeight = fm.height();
        int height = contentRect.bottom() - urlHeight;
        p->setPen( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
        int arrowWidth = ( mShowNextButton ? s_arrowWidth : 0 );
        if ( mShowComicUrl ) {
            p->drawText( QRectF( contentRect.left(), height, contentRect.width() - arrowWidth, fm.height() ),
                         Qt::AlignRight, mWebsiteUrl.host() );
        }

        if ( !mShownIdentifierSuffix.isEmpty() && mShowComicIdentifier ) {
            arrowWidth = ( mShowPreviousButton ? s_arrowWidth : 0 );
            p->drawText( QRectF( contentRect.left() + arrowWidth , height, contentRect.width() - arrowWidth,
                                 fm.height() ), Qt::AlignLeft, mShownIdentifierSuffix );
        }
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    p->setRenderHint( QPainter::SmoothPixmapTransform );

    int leftImageGap = 0;
    int buttonMiddle = ( contentRect.height() / 2 ) + contentRect.top();
    if ( mShowPreviousButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.left() + 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
        p->drawPolygon( arrow );

        leftImageGap = s_arrowWidth;
    }

    int rightImageGap = 0;
    if ( mShowNextButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.right() - 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
        p->drawPolygon( arrow );

        rightImageGap = s_arrowWidth;
    }

    QRect imageRect( contentRect.x() + leftImageGap, contentRect.y() + topHeight,
                     contentRect.width() - ( leftImageGap + rightImageGap ),
                     contentRect.height() - urlHeight - topHeight );
    p->drawImage( imageRect, mImage );

    p->restore();

    toolTipData.mainText += mAdditionalText;
    Plasma::ToolTipManager::self()->setToolTipContent( this, toolTipData );
}

QList<QAction*> ComicApplet::contextualActions()
{
    return mActions;
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    Plasma::DataEngine *engine = dataEngine( "comic" );
    if ( !engine )
        return;

    setCursor( Qt::WaitCursor );
    const QString identifier = mComicIdentifier + ':' + identifierSuffix;

    engine->disconnectSource( identifier, this );
    engine->connectSource( identifier, this );

    const Plasma::DataEngine::Data data = engine->query( identifier );
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
}

void ComicApplet::updateContextMenu()
{
    mActionGoFirst->setVisible( !mFirstDayIdentifierSuffix.isEmpty() );
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

#include "comic.moc"
