/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2011 Matthias Fuchs <mat69@gmx.net>                *
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

#include "comic.h"
#include "comicarchivedialog.h"
#include "comicarchivejob.h"
#include "checknewstrips.h"
#include "stripselector.h"
#include "comicsaver.h"

#include <QtCore/QTimer>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QSortFilterProxyModel>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <KAction>
#include <KConfigDialog>
#include <KDebug>
#include <KNotification>
#include <kuiserverjobtracker.h>
#include <KRun>
#include <KStandardShortcut>

#include <Plasma/Containment>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include "comicmodel.h"
#include "configwidget.h"

K_GLOBAL_STATIC( ComicUpdater, globalComicUpdater )

const int ComicApplet::CACHE_LIMIT = 20;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::PopupApplet( parent, args ),
      mActiveComicModel(parent),
      mDifferentComic( true ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false ),
      mShowErrorPicture( true ),
      mArrowsOnHover( true ),
      mMiddleClick( true ),
      mCheckNewStrips( 0 ),
      mDeclarativeWidget( 0 ),
      mActionShop( 0 ),
      mEngine( 0 ),
      mSavingDir(0)
{
    setHasConfigurationInterface( true );
    resize( 600, 250 );
    setAspectRatioMode( Plasma::IgnoreAspectRatio );

    setPopupIcon( "face-smile-big" );
}

void ComicApplet::init()
{
    globalComicUpdater->init( globalConfig() );
    mSavingDir = new SavingDir(config());

    configChanged();
    //QML
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    mDeclarativeWidget = new Plasma::DeclarativeWidget(this);
    layout->addItem(mDeclarativeWidget);

    mDeclarativeWidget->engine()->rootContext()->setContextProperty("comicApplet", this);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package package(QString(), "org.kde.comic", structure);
    mDeclarativeWidget->setQmlPath(package.filePath("mainscript"));
    //End of QML
    
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
    connect( mActionNextNewStripTab, SIGNAL(triggered(bool)), this, SIGNAL(showNextNewStrip()) );

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
    mActionScaleContent->setChecked( mCurrent.scaleComic() );
    mActions.append( mActionScaleContent );
    connect( mActionScaleContent, SIGNAL(triggered(bool)), this , SLOT(slotScaleToContent()) );

    mActionStorePosition = new QAction( KIcon( "go-home" ), i18nc( "@option:check Context menu of comic image", "Store current &Position" ), this);
    mActionStorePosition->setCheckable( true );
    mActionStorePosition->setChecked(mCurrent.hasStored());
    mActions.append( mActionStorePosition );
    connect( mActionStorePosition, SIGNAL(triggered(bool)), this, SLOT(slotStorePosition()) );

    //make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    updateUsedComics();
    changeComic( true );
}

ComicApplet::~ComicApplet()
{
    delete mSavingDir;
}

QGraphicsWidget *ComicApplet::graphicsWidget()
{
    return mDeclarativeWidget;
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
    }

    mCurrent.setData(data);

    setAssociatedApplicationUrls(mCurrent.websiteUrl());

    //looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if (!mCurrent.hasNext() && mCheckNewComicStripsIntervall) {
        setTabHighlighted( mCurrent.id(), false );
        mActionNextNewStripTab->setEnabled( hasHighlightedTabs() );
    }

    //call the slot to check if the position needs to be saved
    slotStorePosition();

    //disconnect if there is either no error, or an error that can not be fixed automatically
    if ( !errorAutoFixable ) {
        mEngine->disconnectSource( source, this );
    }

    //prefetch the previous and following comic for faster navigation
    if (mCurrent.hasNext()) {
        const QString prefetch = mCurrent.id() + ':' + mCurrent.next();
        mEngine->connectSource( prefetch, this );
        mEngine->query( prefetch );
    }
    if ( mCurrent.hasPrev()) {
        const QString prefetch = mCurrent.id() + ':' + mCurrent.prev();
        mEngine->connectSource( prefetch, this );
        mEngine->query( prefetch );
    }

    updateView();

    refreshComicData();
}

void ComicApplet::updateView()
{
    updateContextMenu();
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

    connect( parent, SIGNAL(applyClicked()), this, SLOT(applyConfig()) );
    connect( parent, SIGNAL(okClicked()), this, SLOT(applyConfig()) );
    connect(mConfigWidget, SIGNAL(enableApply()), parent, SLOT(settingsModified()));
}

void ComicApplet::applyConfig()
{
    setShowComicUrl(mConfigWidget->showComicUrl());
    setShowComicAuthor(mConfigWidget->showComicAuthor());
    setShowComicTitle(mConfigWidget->showComicTitle());
    setShowComicIdentifier(mConfigWidget->showComicIdentifier());
    setShowErrorPicture(mConfigWidget->showErrorPicture());
    setArrowsOnHover(mConfigWidget->arrowsOnHover());
    setMiddleClick(mConfigWidget->middleClick());
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

    //make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    changeComic( mDifferentComic );
}

void ComicApplet::changeComic( bool differentComic )
{
    if ( differentComic ) {
        KConfigGroup cg = config();
        mActionStorePosition->setChecked(mCurrent.storePosition());

        // assign mScaleComic the moment the new strip has been loaded (dataUpdated) as up to this point
        // the old one should be still shown with its scaling settings
        mActionScaleContent->setChecked( mCurrent.scaleComic() );

        updateComic( mCurrent.stored() );
    } else {
        updateComic( mCurrent.current() );
    }
}

void ComicApplet::updateUsedComics()
{
    const QString oldIdentifier = mCurrent.id();

    mActiveComicModel.clear();
    mTabIdentifier.clear();
    mCurrent = ComicData();

    bool isFirst = true;
    QModelIndex data;
    KConfigGroup cg = config();
    int tab = 0;
    for ( int i = 0; i < mProxy->rowCount(); ++i ) {
        if ( mProxy->index( i, 0 ).data( Qt::CheckStateRole) == Qt::Checked ) {
            data = mProxy->index( i, 1 );

            if ( isFirst ) {
                isFirst = false;
                const QString id = data.data( Qt::UserRole ).toString();
                mDifferentComic = ( oldIdentifier != id );
                const QString title = data.data().toString();
                mCurrent.init(id, config());
                mCurrent.setTitle(title);
            }

            const QString name = data.data().toString();
            const QString identifier = data.data( Qt::UserRole ).toString();
            const QString iconPath = data.data( Qt::DecorationRole ).value<QIcon>().name();
            //found a newer strip last time, which was not visited
            if ( mCheckNewComicStripsIntervall && !cg.readEntry( "lastStripVisited_" + identifier, true ) ) {
                mActiveComicModel.addComic(identifier, name, iconPath, true);
            } else {
                mActiveComicModel.addComic(identifier, name, iconPath);
            }

            mTabIdentifier << identifier;
            ++tab;
        }
    }

    mActionNextNewStripTab->setVisible( mCheckNewComicStripsIntervall );
    mActionNextNewStripTab->setEnabled( hasHighlightedTabs() );

    delete mCheckNewStrips;
    mCheckNewStrips = 0;
    if ( mCheckNewComicStripsIntervall ) {
        mCheckNewStrips = new CheckNewStrips( mTabIdentifier, mEngine, mCheckNewComicStripsIntervall, this );
        connect( mCheckNewStrips, SIGNAL(lastStrip(int,QString,QString)), this, SLOT(slotFoundLastStrip(int,QString,QString)) );
    }

    emit comicModelChanged();
}

void ComicApplet::slotTabChanged(const QString &identifier)
{
    bool differentComic = (mCurrent.id() != identifier);
    mCurrent = ComicData();
    mCurrent.init(identifier, config());
    changeComic( differentComic );
}

void ComicApplet::checkDayChanged()
{
    if ( ( mCurrentDay != QDate::currentDate() ) || !mCurrent.hasImage() )
        updateComic( mCurrent.stored() );

    mCurrentDay = QDate::currentDate();
}

void ComicApplet::configChanged()
{
    KConfigGroup cg = config();
    mTabIdentifier = cg.readEntry( "tabIdentifier", QStringList( QString() ) );

    const QString id = mTabIdentifier.count() ? mTabIdentifier.at( 0 ) : QString();
    mCurrent = ComicData();
    mCurrent.init(id, cg);

    mShowComicUrl = cg.readEntry( "showComicUrl", false );
    mShowComicAuthor = cg.readEntry( "showComicAuthor", false );
    mShowComicTitle = cg.readEntry( "showComicTitle", false );
    mShowComicIdentifier = cg.readEntry( "showComicIdentifier", false );
    mShowErrorPicture = cg.readEntry( "showErrorPicture", true );
    mArrowsOnHover = cg.readEntry( "arrowsOnHover", true );
    mMiddleClick = cg.readEntry( "middleClick", true );
    mCheckNewComicStripsIntervall = cg.readEntry( "checkNewComicStripsIntervall", 30 );

    //use a decent default size
    const QSizeF tempMaxSize = isInPanel() ? QSizeF( 600, 250 ) : this->size();
    mMaxSize = cg.readEntry( "maxSize", tempMaxSize );
    mLastSize = mMaxSize;

    globalComicUpdater->load();
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mCurrent.id() );
    cg.writeEntry( "showComicUrl", mShowComicUrl );
    cg.writeEntry( "showComicAuthor", mShowComicAuthor );
    cg.writeEntry( "showComicTitle", mShowComicTitle );
    cg.writeEntry( "showComicIdentifier", mShowComicIdentifier );
    cg.writeEntry( "showErrorPicture", mShowErrorPicture );
    cg.writeEntry( "arrowsOnHover", mArrowsOnHover );
    cg.writeEntry( "middleClick", mMiddleClick );
    cg.writeEntry( "tabIdentifier", mTabIdentifier );
    cg.writeEntry( "checkNewComicStripsIntervall", mCheckNewComicStripsIntervall );

    globalComicUpdater->save();
}

void ComicApplet::slotNextDay()
{
    updateComic(mCurrent.next());
}

void ComicApplet::slotPreviousDay()
{
    updateComic(mCurrent.prev());
}

void ComicApplet::slotFirstDay()
{
    updateComic(mCurrent.first());
}

void ComicApplet::slotCurrentDay()
{
    updateComic(QString());
}

void ComicApplet::slotFoundLastStrip( int index, const QString &identifier, const QString &suffix )
{
    KConfigGroup cg = config();
    if ( suffix != cg.readEntry( "lastStrip_" + identifier, QString() ) ) {
        kDebug() << identifier << "has a newer strip.";
        setTabHighlighted( identifier, true );
        cg.writeEntry( "lastStripVisited_" + identifier, false );
    }

    mActionNextNewStripTab->setEnabled( hasHighlightedTabs() );
}

void ComicApplet::slotGoJump()
{
    StripSelector *selector = StripSelectorFactory::create(mCurrent.type());
    connect(selector, SIGNAL(stripChosen(QString)), this, SLOT(updateComic(QString)));

    selector->select(mCurrent);
}

void ComicApplet::slotStorePosition()
{
    mCurrent.storePosition(mActionStorePosition->isChecked());
}

void ComicApplet::slotShop()
{
    KRun::runUrl(mCurrent.shopUrl(), "text/html", 0);
}

bool ComicApplet::isInPanel() const
{
    return ( this->geometry().width() < 70 ) || ( this->geometry().height() < 50 );
}

void ComicApplet::createComicBook()
{
    ComicArchiveDialog *dialog = new ComicArchiveDialog(mCurrent.id(), mCurrent.title(), mCurrent.type(), mCurrent.current(),
                                                        mCurrent.first(), mSavingDir->getDir());
    dialog->setAttribute(Qt::WA_DeleteOnClose);//to have destroyed emitted upon closing
    connect( dialog, SIGNAL(archive(int,KUrl,QString,QString)), this, SLOT(slotArchive(int,KUrl,QString,QString)) );
    dialog->show();
}

void ComicApplet::slotArchive( int archiveType, const KUrl &dest, const QString &fromIdentifier, const QString &toIdentifier )
{
    mSavingDir->setDir(dest.directory());

    const QString id = mCurrent.id();
    kDebug() << "Archiving:" << id <<  archiveType << dest << fromIdentifier << toIdentifier;
    ComicArchiveJob *job = new ComicArchiveJob(dest, mEngine, static_cast< ComicArchiveJob::ArchiveType >( archiveType ), mCurrent.type(),  id, this);
    job->setFromIdentifier(id + ':' + fromIdentifier);
    job->setToIdentifier(id + ':' + toIdentifier);
    if (job->isValid()) {
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotArchiveFinished(KJob*)));
        KIO::getJobTracker()->registerJob(job);
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
        QSize imageSize = mCurrent.image().size();
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
    const QString id = mCurrent.id();
    setConfigurationRequired( id.isEmpty() );
    if ( !id.isEmpty() && mEngine && mEngine->isValid() ) {

        setBusy( true );
        const QString identifier = id + ':' + identifierSuffix;

        //disconnecting of the oldSource is needed, otherwise you could get data for comics you are not looking at if you use tabs
        //if there was an error only disconnect the oldSource if it had nothing to do with the error or if the comic changed, that way updates of the error can come in
        if ( !mIdentifierError.isEmpty() && !mIdentifierError.contains( id ) ) {
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
        kError() << "Either no identifier was specified or the engine could not be created:" << "id" << id << "engine valid:" << ( mEngine && mEngine->isValid() );
        setConfigurationRequired( true );
    }
}

void ComicApplet::updateContextMenu()
{
    mActionGoFirst->setVisible(mCurrent.hasFirst());
    mActionGoFirst->setEnabled(mCurrent.hasPrev());
    mActionGoLast->setEnabled(true);//always enable to have some kind of refresh action
    if (mActionShop) {
        mActionShop->setEnabled(mCurrent.shopUrl().isValid());
    }
}

void ComicApplet::slotSaveComicAs()
{
    ComicSaver saver(mSavingDir);
    saver.save(mCurrent);
}

void ComicApplet::slotScaleToContent()
{
    setShowActualSize(mActionScaleContent->isChecked());
}

//QML
QObject *ComicApplet::comicsModel() 
{
    return &mActiveComicModel;
}

bool ComicApplet::showComicUrl() const
{
    return mShowComicUrl;
}

void ComicApplet::setShowComicUrl(bool show)
{
    if (show == mShowComicUrl)
        return;

    mShowComicUrl = show;

    emit showComicUrlChanged();
}

bool ComicApplet::showComicAuthor() const
{
    return mShowComicAuthor;
}

void ComicApplet::setShowComicAuthor(bool show)
{
    if (show == mShowComicAuthor)
        return;

    mShowComicAuthor = show;

    emit showComicAuthorChanged();
}

bool ComicApplet::showComicTitle() const
{
    return mShowComicTitle;
}

void ComicApplet::setShowComicTitle(bool show)
{
    if (show == mShowComicTitle)
        return;

    mShowComicTitle = show;

    emit showComicTitleChanged();
}

bool ComicApplet::showComicIdentifier() const
{
    return mShowComicIdentifier;
}

void ComicApplet::setShowComicIdentifier(bool show)
{
    if (show == mShowComicIdentifier)
        return;

    mShowComicIdentifier = show;

    emit showComicIdentifierChanged();
}

bool ComicApplet::showErrorPicture() const
{
    return mShowErrorPicture;
}

void ComicApplet::setShowErrorPicture(bool show)
{
    if (show == mShowErrorPicture)
        return;

    mShowErrorPicture = show;

    emit showErrorPictureChanged();
}

bool ComicApplet::arrowsOnHover() const
{
    return mArrowsOnHover;
}

void ComicApplet::setArrowsOnHover(bool show)
{
    if (show == mArrowsOnHover)
        return;

    mArrowsOnHover = show;

    emit arrowsOnHoverChanged();
}

bool ComicApplet::middleClick() const
{
    return mMiddleClick;
}

void ComicApplet::setMiddleClick(bool show)
{
    if (show == mMiddleClick)
        return;

    mMiddleClick = show;

    emit middleClickChanged();
}

QVariantHash ComicApplet::comicData()
{
    return mComicData;
}

void ComicApplet::refreshComicData()
{
    mComicData["image"] = mCurrent.image();
    mComicData["prev"] = mCurrent.prev();
    mComicData["next"] = mCurrent.next();
    mComicData["additionalText"] = mCurrent.additionalText();

    mComicData["websiteUrl"] = mCurrent.websiteUrl().prettyUrl();
    mComicData["websiteHost"] = mCurrent.websiteUrl().host();
    mComicData["imageUrl"] = mCurrent.websiteUrl().prettyUrl();
    mComicData["shopUrl"] = mCurrent.websiteUrl().prettyUrl();
    mComicData["first"] = mCurrent.first();
    mComicData["stripTitle"] = mCurrent.stripTitle();
    mComicData["author"] = mCurrent.author();
    mComicData["title"] = mCurrent.title();

    mComicData["suffixType"] = "Date";
    mComicData["current"] = mCurrent.current();
    //mComicData["last"] = mCurrent.last();
    mComicData["currentReadable"] = mCurrent.currentReadable();
    mComicData["firstStripNum"] = mCurrent.firstStripNum();
    mComicData["maxStripNum"] = mCurrent.maxStripNum();
    mComicData["isLeftToRight"] = mCurrent.isLeftToRight();
    mComicData["isTopToBottom"] = mCurrent.isTopToBottom();

    emit comicDataChanged();
}

bool ComicApplet::showActualSize() const
{
    return mCurrent.scaleComic();
}

void ComicApplet::setShowActualSize(bool show)
{
    if (show == mCurrent.scaleComic())
        return;

    mCurrent.setScaleComic(show);

    emit showActualSizeChanged();
}
//Endof QML
void ComicApplet::setTabHighlighted(const QString &id, bool highlight)
{
    //Search for matching id
    for (int index = 0; index < mActiveComicModel.rowCount(); ++index) {
        QStandardItem * item = mActiveComicModel.item(index);

        QString currentId = item->data(ActiveComicModel::ComicKeyRole).toString();
        if (id == currentId){
            if (highlight != item->data(ActiveComicModel::ComicHighlightRole).toBool()) {
                item->setData(highlight, ActiveComicModel::ComicHighlightRole);
                emit tabHighlightRequest(id, highlight);
            }
        }
    }
}

bool ComicApplet::hasHighlightedTabs()
{
    for (int i = 0; i < mActiveComicModel.rowCount(); ++i) {
        if (isTabHighlighted(i)) {
            return true;
        }
    }

    return false;
}

bool ComicApplet::isTabHighlighted(int index) const
{
    if (index < 0 || index >= mActiveComicModel.rowCount()) {
        return false;
    }

    QStandardItem * item = mActiveComicModel.item(index);

    return item->data(ActiveComicModel::ComicHighlightRole).toBool();
}
#include "comic.moc"
