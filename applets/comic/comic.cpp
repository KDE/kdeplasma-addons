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
#include <QtGui/QPainter>

#include <KConfigDialog>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <KRun>
#include <KTemporaryFile>

#include <Plasma/Theme>
#include <plasma/tooltipmanager.h>

#include "configwidget.h"
#include "fullviewwidget.h"
#include "pluginmanager.h"

static const int s_arrowWidth = 30;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mCurrentDate( QDate::currentDate() ),
      mShowPreviousButton( true ),
      mShowNextButton( false ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false )
{
    setHasConfigurationInterface( true );
    resize( 480, 160 );
    setAspectRatioMode( Plasma::KeepAspectRatio );
}

void ComicApplet::init()
{
    Plasma::ToolTipManager::self()->registerWidget(this);
    loadConfig();

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, SIGNAL( timeout() ), this, SLOT( checkDayChanged() ) );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes

    Solid::Networking::Status status = Solid::Networking::status();
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown )
        updateComic();

    connect( Solid::Networking::notifier(), SIGNAL( statusChanged( Solid::Networking::Status ) ),
             this, SLOT( networkStatusChanged( Solid::Networking::Status ) ) );

    updateButtons();

    QAction *action = new QAction( KIcon( "document-save-as" ), i18n( "&Save Comic As..." ), this );
    mActions.append( action );
    connect( action, SIGNAL( triggered( bool ) ), this , SLOT( slotSaveComicAs() ) );

    mFullViewWidget = new FullViewWidget();
    mFullViewWidget->hide();
}

ComicApplet::~ComicApplet()
{
}

void ComicApplet::dataUpdated( const QString&, const Plasma::DataEngine::Data &data )
{
    setCursor( Qt::ArrowCursor );

    mImage = data[ "Image" ].value<QImage>();
    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mAdditionalText = data[ "Additional text" ].toString();
    mComicAuthor = PluginManager::Instance()->comicAuthor( mComicIdentifier );
    mComicTitle = PluginManager::Instance()->comicTitle( mComicIdentifier );

    updateButtons();

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

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage( mConfigWidget, parent->windowTitle(), icon() );

    connect( parent, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
}

void ComicApplet::applyConfig()
{
    mComicIdentifier = mConfigWidget->comicIdentifier();
    mShowComicUrl = mConfigWidget->showComicUrl();
    mShowComicAuthor = mConfigWidget->showComicAuthor();
    mShowComicTitle = mConfigWidget->showComicTitle();

    saveConfig();

    updateComic();
}

void ComicApplet::networkStatusChanged( Solid::Networking::Status status )
{
    if ( status == Solid::Networking::Connected )
        updateComic();
}

void ComicApplet::checkDayChanged()
{
    if ( mCurrentDay != QDate::currentDate() || mImage.isNull() )
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
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
    cg.writeEntry( "showComicUrl", mShowComicUrl );
    cg.writeEntry( "showComicAuthor", mShowComicAuthor );
    cg.writeEntry( "showComicTitle", mShowComicTitle );
}

void ComicApplet::slotNextDay()
{
    updateComic( mNextIdentifierSuffix );
}

void ComicApplet::slotPreviousDay()
{
    updateComic( mPreviousIdentifierSuffix );
}

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();

        const QRectF rect = contentsRect();
        if ( mShowPreviousButton && event->pos().x() > rect.left() &&
                                    event->pos().x() < (rect.left() + s_arrowWidth) ) {
            slotPreviousDay();
        } else if ( mShowNextButton && event->pos().x() > (rect.right() - s_arrowWidth) &&
                                       event->pos().x() < rect.right() ) {
            slotNextDay();
        } else if ( !mWebsiteUrl.isEmpty() &&
                    event->pos().y() > (rect.bottom() - fm.height()) &&
                    event->pos().x() > (rect.right() - fm.width( mWebsiteUrl.host() ) - s_arrowWidth) ) {
            // link clicked
            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
        }
    } else if ( event->button() == Qt::MidButton ) {
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
        qreal aspectRatio = qreal(size.height()) / size.width();
        qreal imageHeight = aspectRatio * ( geometry().width() - leftArea - rightArea );
        int fmHeight = Plasma::Theme::defaultTheme()->fontMetrics().height();
        int topArea = ( ( mShowComicAuthor || mShowComicTitle ) ? fmHeight : 0 );
        int bottomArea = ( mShowComicUrl ? fmHeight : 0 );
        resize( geometry().width(), imageHeight + topArea + bottomArea );
    }
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect &contentRect )
{
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

    int topHeight = 0;
    if ( ( mShowComicAuthor || mShowComicTitle ) && !tempTop.isEmpty() ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        topHeight = fm.height();
        int height = contentRect.top();
        p->setPen( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawText( QRectF( contentRect.left(), height, contentRect.width(), fm.height() ),
                    Qt::AlignCenter, tempTop );
    }

    int urlHeight = 0;
    if ( !mWebsiteUrl.isEmpty() && mShowComicUrl ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        urlHeight = fm.height();
        int height = contentRect.bottom() - urlHeight;
        p->setPen( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawText( QRectF( contentRect.left(), height, contentRect.width(), fm.height() ),
                     Qt::AlignRight, mWebsiteUrl.host() );
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    p->setRenderHint( QPainter::SmoothPixmapTransform );

    int leftImageGap = 0;
    int buttonMiddle = (contentRect.height() / 2) + contentRect.top();
    if ( mShowPreviousButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.left() + 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawPolygon( arrow );

        leftImageGap = s_arrowWidth;
    }

    int rightImageGap = 0;
    if ( mShowNextButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.right() - 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawPolygon( arrow );

        rightImageGap = s_arrowWidth;
    }

    QRect imageRect( contentRect.x() + leftImageGap, contentRect.y() + topHeight,
                     contentRect.width() - (leftImageGap + rightImageGap), contentRect.height() - urlHeight - topHeight );
    p->drawImage( imageRect, mImage );

    p->restore();
    toolTipData.mainText += mAdditionalText;
    Plasma::ToolTipManager::self()->setToolTipContent(this,toolTipData);
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
    if ( mNextIdentifierSuffix.isNull() )
        mShowNextButton = false;
    else
        mShowNextButton = true;

    if ( mPreviousIdentifierSuffix.isNull() )
        mShowPreviousButton = false;
    else
        mShowPreviousButton = true;
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
