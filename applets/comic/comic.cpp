/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2011 Matthias Fuchs <mat69@gmx.net>                *
 *   Copyright (C) 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>      *
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
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

#include <QTimer>
#include <QSortFilterProxyModel>

#include <QAction>
#include <QDebug>
#include <KActionCollection>
#include <KConfigDialog>
#include <KNotification>
#include <kuiserverjobtracker.h>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KRun>
#include <KStandardShortcut>
#include <KIconLoader>

#include <Plasma/Containment>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include "comicmodel.h"
#include "comicupdater.h"

Q_GLOBAL_STATIC( ComicUpdater, globalComicUpdater )

const int ComicApplet::CACHE_LIMIT = 20;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mProxy(0),
      mActiveComicModel(new ActiveComicModel(parent)),
      mDifferentComic( true ),
      mShowComicUrl( false ),
      mShowComicAuthor( false ),
      mShowComicTitle( false ),
      mShowComicIdentifier( false ),
      mShowErrorPicture( true ),
      mArrowsOnHover( true ),
      mMiddleClick( true ),
      mCheckNewComicStripsInterval(0),
      mMaxComicLimit( CACHE_LIMIT ),
      mCheckNewStrips( 0 ),
      mActionShop( 0 ),
      mEngine( 0 ),
      mSavingDir(0)
{
    setHasConfigurationInterface( true );
}

void ComicApplet::init()
{
    globalComicUpdater->init( globalConfig() );
    mSavingDir = new SavingDir(config());

    configChanged();

    mEngine = Plasma::PluginLoader::self()->loadDataEngine( "comic" );
    mModel = new ComicModel( mEngine, "providers", mTabIdentifier, this );
    mProxy = new QSortFilterProxyModel( this );
    mProxy->setSourceModel( mModel );
    mProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
    mProxy->sort( 1, Qt::AscendingOrder );

    //set maximum number of cached strips per comic, -1 means that there is no limit
    KConfigGroup global = globalConfig();
    const int maxComicLimit = global.readEntry( "maxComicLimit", CACHE_LIMIT );
    mEngine->connectSource( QLatin1String( "setting_maxComicLimit:" ) + QString::number( maxComicLimit ), this );

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer( this );
    connect( mDateChangedTimer, &QTimer::timeout, this, &ComicApplet::checkDayChanged );
    mDateChangedTimer->setInterval( 5 * 60 * 1000 ); // every 5 minutes
    mDateChangedTimer->start();

    mActionNextNewStripTab = new QAction( QIcon::fromTheme( "go-next-view" ), i18nc( "here strip means comic strip", "&Next Tab with a new Strip" ), this );
    mActionNextNewStripTab->setShortcuts( KStandardShortcut::openNew() );
    actions()->addAction( "next new strip" , mActionNextNewStripTab );
    mActions.append( mActionNextNewStripTab );
    connect( mActionNextNewStripTab, &QAction::triggered, this, &ComicApplet::showNextNewStrip );

    mActionGoFirst = new QAction( QIcon::fromTheme( "go-first" ), i18n( "Jump to &first Strip" ), this );
    mActions.append( mActionGoFirst );
    connect( mActionGoFirst, &QAction::triggered, this, &ComicApplet::slotFirstDay );

    mActionGoLast = new QAction( QIcon::fromTheme( "go-last" ), i18n( "Jump to &current Strip" ), this );
    mActions.append( mActionGoLast );
    connect( mActionGoLast, &QAction::triggered, this, &ComicApplet::slotCurrentDay );

    mActionGoJump = new QAction( QIcon::fromTheme( "go-jump" ), i18n( "Jump to Strip ..." ), this );
    mActions.append( mActionGoJump );
    connect( mActionGoJump, &QAction::triggered, this, &ComicApplet::slotGoJump );

    mActionShop = new QAction( i18n( "Visit the shop &website" ), this );
    mActionShop->setEnabled( false );
    mActions.append( mActionShop );
    connect( mActionShop, &QAction::triggered, this, &ComicApplet::slotShop );

    QAction *action = new QAction( QIcon::fromTheme( "document-save-as" ), i18n( "&Save Comic As..." ), this );
    mActions.append( action );
    connect( action, &QAction::triggered, this, &ComicApplet::slotSaveComicAs );

    action = new QAction( QIcon::fromTheme( "application-epub+zip" ), i18n( "&Create Comic Book Archive..." ), this );
    mActions.append( action );
    connect( action, &QAction::triggered, this, &ComicApplet::createComicBook );

    mActionScaleContent = new QAction( QIcon::fromTheme( "zoom-original" ), i18nc( "@option:check Context menu of comic image", "&Actual Size" ), this );
    mActionScaleContent->setCheckable( true );
    mActionScaleContent->setChecked( mCurrent.scaleComic() );
    mActions.append( mActionScaleContent );
    connect( mActionScaleContent, &QAction::triggered, this, &ComicApplet::slotScaleToContent );

    mActionStorePosition = new QAction( QIcon::fromTheme( "go-home" ), i18nc( "@option:check Context menu of comic image", "Store current &Position" ), this);
    mActionStorePosition->setCheckable( true );
    mActionStorePosition->setChecked(mCurrent.hasStored());
    mActions.append( mActionStorePosition );
    connect( mActionStorePosition, &QAction::triggered, this, &ComicApplet::slotStorePosition );

    //make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    updateUsedComics();
    changeComic( true );
}

ComicApplet::~ComicApplet()
{
    if (mEngine && mEngine->isValid()) {
        auto sources = mEngine->sources();
        Q_FOREACH(auto source, sources) {
            mEngine->disconnectSource(source, this);
        }
    }
    delete mSavingDir;
}

void ComicApplet::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
    if (graphicObject) {
        graphicObject->setProperty("busy", false);
    }

    //disconnect prefetched comic strips
    if ( source != mOldSource ) {
        mEngine->disconnectSource( source, this );
        return;
    }

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

    setAssociatedApplicationUrls(QList<QUrl>() << mCurrent.websiteUrl());

    //looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if (!mCurrent.hasNext() && mCheckNewComicStripsInterval) {
        setTabHighlighted( mCurrent.id(), false );
        mActionNextNewStripTab->setEnabled( isTabHighlighted(mCurrent.id()) );
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
    }
    if ( mCurrent.hasPrev()) {
        const QString prefetch = mCurrent.id() + ':' + mCurrent.prev();
        mEngine->connectSource( prefetch, this );
    }

    updateView();

    refreshComicData();
}

void ComicApplet::updateView()
{
    updateContextMenu();
}

void ComicApplet::getNewComics()
{
    if (!m_newStuffDialog) {
        m_newStuffDialog = new KNS3::DownloadDialog( QString::fromLatin1("comic.knsrc") );
        KNS3::DownloadDialog *strong = m_newStuffDialog.data();
        strong->setTitle(i18n("Download Comics"));
        connect(m_newStuffDialog.data(), SIGNAL(finished(int)), mEngine, SLOT(loadProviders()));
    }
    m_newStuffDialog.data()->show();
}

void ComicApplet::positionFullView(QWindow *window)
{
    if (!window || !window->screen()) {
        return;
    }

    window->setPosition(window->screen()->availableGeometry().center() - QPoint(window->size().width()/2, window->size().height()/2));
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

    mActiveComicModel->clear();
    mCurrent = ComicData();

    bool isFirst = true;
    QModelIndex data;
    KConfigGroup cg = config();
    int tab = 0;
    for ( int i = 0; i < mProxy->rowCount(); ++i ) {
        if (mTabIdentifier.contains(mProxy->index( i, 0 ).data( Qt::UserRole).toString())) {
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

            if ( mCheckNewComicStripsInterval && !cg.readEntry( "lastStripVisited_" + identifier, true ) ) {
                mActiveComicModel->addComic(identifier, name, iconPath, true);
            } else {
                mActiveComicModel->addComic(identifier, name, iconPath);
            }

            ++tab;
        }
    }

    mActionNextNewStripTab->setVisible( mCheckNewComicStripsInterval );
    mActionNextNewStripTab->setEnabled( isTabHighlighted(mCurrent.id()) );

    delete mCheckNewStrips;
    mCheckNewStrips = 0;
    if ( mCheckNewComicStripsInterval ) {
        mCheckNewStrips = new CheckNewStrips( mTabIdentifier, mEngine, mCheckNewComicStripsInterval, this );
        connect( mCheckNewStrips, &CheckNewStrips::lastStrip, this, &ComicApplet::slotFoundLastStrip );
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
    mTabIdentifier = cg.readEntry( "tabIdentifier", QStringList() );

    if (mProxy) {
        updateUsedComics();
    }

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
    mCheckNewComicStripsInterval = cg.readEntry( "checkNewComicStripsIntervall", 30 );
    KConfigGroup global = globalConfig();
    mMaxComicLimit = global.readEntry( "maxComicLimit", CACHE_LIMIT );

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
    cg.writeEntry( "checkNewComicStripsIntervall", mCheckNewComicStripsInterval );
    cg.writeEntry( "maxComicLimit", mMaxComicLimit);

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
    Q_UNUSED(index)

    KConfigGroup cg = config();
    if (suffix != cg.readEntry( "lastStrip_" + identifier, QString() ) ) {
        qDebug() << identifier << "has a newer strip.";
        cg.writeEntry( "lastStripVisited_" + identifier, false );
        updateComic(suffix);
    }
}

void ComicApplet::slotGoJump()
{
    StripSelector *selector = StripSelectorFactory::create(mCurrent.type());
    connect(selector, &StripSelector::stripChosen, this, &ComicApplet::updateComic);

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

void ComicApplet::createComicBook()
{
    ComicArchiveDialog *dialog = new ComicArchiveDialog(mCurrent.id(), mCurrent.title(), mCurrent.type(), mCurrent.current(),
                                                        mCurrent.first(), mSavingDir->getDir());
    dialog->setAttribute(Qt::WA_DeleteOnClose);//to have destroyed emitted upon closing
    connect( dialog, &ComicArchiveDialog::archive, this, &ComicApplet::slotArchive );
    dialog->show();
}

void ComicApplet::slotArchive( int archiveType, const QUrl &dest, const QString &fromIdentifier, const QString &toIdentifier )
{
    mSavingDir->setDir(dest.path());

    const QString id = mCurrent.id();
    qDebug() << "Archiving:" << id <<  archiveType << dest << fromIdentifier << toIdentifier;
    ComicArchiveJob *job = new ComicArchiveJob(dest, mEngine, static_cast< ComicArchiveJob::ArchiveType >( archiveType ), mCurrent.type(),  id, this);
    job->setFromIdentifier(id + ':' + fromIdentifier);
    job->setToIdentifier(id + ':' + toIdentifier);
    if (job->isValid()) {
        connect(job, &ComicArchiveJob::finished, this, &ComicApplet::slotArchiveFinished);
        KIO::getJobTracker()->registerJob(job);
        job->start();
    } else {
        qWarning() << "Archiving job is not valid.";
        delete job;
    }
}

void ComicApplet::slotArchiveFinished (KJob *job )
{
    if ( job->error() ) {
        KNotification::event( KNotification::Warning, i18n( "Archiving comic failed" ), job->errorText(), QIcon::fromTheme( "dialog-warning" ).pixmap( KIconLoader::SizeMedium ) );
    }
}

QList<QAction*> ComicApplet::contextualActions()
{
    return mActions;
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    const QString id = mCurrent.id();
    setConfigurationRequired( id.isEmpty() );

    if ( !id.isEmpty() && mEngine && mEngine->isValid() ) {

        QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", true);
        }
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
        mEngine->connectSource( identifier, this );
        slotScaleToContent();
    } else {
        qWarning() << "Either no identifier was specified or the engine could not be created:" << "id" << id << "engine valid:" << ( mEngine && mEngine->isValid() );
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
QObject *ComicApplet::comicsModel() const
{
    return mActiveComicModel;
}

QObject *ComicApplet::availableComicsModel() const
{
    return mProxy;
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
    saveConfig();
}

QVariantMap ComicApplet::comicData() const
{
    return mComicData;
}

QStringList ComicApplet::tabIdentifiers() const
{
    return mTabIdentifier;
}

void ComicApplet::setTabIdentifiers(const QStringList &tabs)
{
    if (mTabIdentifier == tabs) {
        return;
    }

    mTabIdentifier = tabs;
    emit tabIdentifiersChanged();
    saveConfig();
    changeComic( mDifferentComic );
}

void ComicApplet::refreshComicData()
{
    mComicData["image"] = mCurrent.image();
    mComicData["prev"] = mCurrent.prev();
    mComicData["next"] = mCurrent.next();
    mComicData["additionalText"] = mCurrent.additionalText();

    mComicData["websiteUrl"] = mCurrent.websiteUrl().toString();
    mComicData["websiteHost"] = mCurrent.websiteUrl().host();
    mComicData["imageUrl"] = mCurrent.websiteUrl().toString();
    mComicData["shopUrl"] = mCurrent.websiteUrl().toString();
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

int ComicApplet::checkNewComicStripsInterval() const
{
    return mCheckNewComicStripsInterval;
}

void ComicApplet::setCheckNewComicStripsInterval(int interval)
{
    if (mCheckNewComicStripsInterval == interval) {
        return;
    }

    mCheckNewComicStripsInterval = interval;
    emit checkNewComicStripsIntervalChanged();
}

int ComicApplet::providerUpdateInterval() const
{
    return globalComicUpdater->interval();
}

void ComicApplet::setProviderUpdateInterval(int interval)
{
    if (globalComicUpdater->interval() == interval) {
        return;
    }

    globalComicUpdater->setInterval(interval);
    emit providerUpdateIntervalChanged();
}

void ComicApplet::setMaxComicLimit(int limit)
{
    if (mMaxComicLimit == limit) {
        return;
    }

    mMaxComicLimit = limit;
    emit maxComicLimitChanged();
}

int ComicApplet::maxComicLimit() const
{
    return mMaxComicLimit;
}

//Endof QML
void ComicApplet::setTabHighlighted(const QString &id, bool highlight)
{
    //Search for matching id
    for (int i = 0; i < mActiveComicModel->rowCount(); ++i) {
        QStandardItem *item = mActiveComicModel->item(i);

        QString currentId = item->data(ActiveComicModel::ComicKeyRole).toString();
        if (id == currentId) {
            if (highlight != item->data(ActiveComicModel::ComicHighlightRole).toBool()) {
                item->setData(highlight, ActiveComicModel::ComicHighlightRole);
                emit tabHighlightRequest(id, highlight);
            }
        }
    }
}

bool ComicApplet::isTabHighlighted(const QString &id) const
{
    for (int i = 0; i < mActiveComicModel->rowCount(); ++i) {
        QStandardItem *item = mActiveComicModel->item(i);

        QString currentId = item->data(ActiveComicModel::ComicKeyRole).toString();
        if (id == currentId) {
            return item->data(ActiveComicModel::ComicHighlightRole).toBool();
        }
    }
    return false;
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(comic, ComicApplet, "metadata.json")

#include "comic.moc"
