/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2008-2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comic.h"
#include "checknewstrips.h"
#include "comic_debug.h"
#include "comicarchivedialog.h"
#include "comicarchivejob.h"
#include "comicdata.h"
#include "comicsaver.h"
#include "stripselector.h"

#include <QAction>
#include <QDebug>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QWindow>

#include <KActionCollection>
#include <KConfigDialog>
#include <KIO/OpenUrlJob>
#include <KNotification>
#include <KStandardShortcut>
#include <kuiserverjobtracker.h>

#include <Plasma/Containment>

#include "comicmodel.h"
#include "comicupdater.h"

Q_GLOBAL_STATIC(ComicUpdater, globalComicUpdater)

ComicApplet::ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
    , mProxy(nullptr)
    , mActiveComicModel(new ActiveComicModel(parent))
    , mDifferentComic(true)
    , mShowComicUrl(false)
    , mShowComicAuthor(false)
    , mShowComicTitle(false)
    , mShowComicIdentifier(false)
    , mShowErrorPicture(true)
    , mArrowsOnHover(true)
    , mMiddleClick(true)
    , mCheckNewComicStripsInterval(0)
    , mMaxComicLimit(0)
    , mCheckNewStrips(nullptr)
    , mActionShop(nullptr)
    , mEngine(nullptr)
    , mSavingDir(nullptr)
{
    setHasConfigurationInterface(true);
}

void ComicApplet::init()
{
    globalComicUpdater->init(globalConfig());
    mSavingDir = new SavingDir(config());

    configChanged();

    mEngine = new ComicEngine(this);
    connect(mEngine, &ComicEngine::requestFinished, this, &ComicApplet::dataUpdated);
    mModel = new ComicModel(mEngine, mTabIdentifier, this);
    mProxy = new QSortFilterProxyModel(this);
    mProxy->setSourceModel(mModel);
    mProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxy->sort(1, Qt::AscendingOrder);

    mCurrentDay = QDate::currentDate();
    mDateChangedTimer = new QTimer(this);
    connect(mDateChangedTimer, &QTimer::timeout, this, &ComicApplet::checkDayChanged);
    mDateChangedTimer->setInterval(5 * 60 * 1000); // every 5 minutes
    mDateChangedTimer->start();

    mActionNextNewStripTab = new QAction(QIcon::fromTheme(QStringLiteral("go-next-view")), i18nc("@action comic strip", "&Next Tab with a New Strip"), this);
    mActionNextNewStripTab->setShortcuts(KStandardShortcut::openNew());
    actions()->addAction(QStringLiteral("next new strip"), mActionNextNewStripTab);
    mActions.append(mActionNextNewStripTab);
    connect(mActionNextNewStripTab, &QAction::triggered, this, &ComicApplet::showNextNewStrip);

    mActionGoFirst = new QAction(QIcon::fromTheme(QStringLiteral("go-first")), i18nc("@action", "Jump to &First Strip"), this);
    mActions.append(mActionGoFirst);
    connect(mActionGoFirst, &QAction::triggered, this, &ComicApplet::slotFirstDay);

    mActionGoLast = new QAction(QIcon::fromTheme(QStringLiteral("go-last")), i18nc("@action", "Jump to &Current Strip"), this);
    mActions.append(mActionGoLast);
    connect(mActionGoLast, &QAction::triggered, this, &ComicApplet::slotCurrentDay);

    mActionGoJump = new QAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18nc("@action", "Jump to Strip…"), this);
    mActions.append(mActionGoJump);
    connect(mActionGoJump, &QAction::triggered, this, &ComicApplet::slotGoJump);

    mActionShop = new QAction(i18nc("@action", "Visit the Shop &Website"), this);
    mActionShop->setEnabled(false);
    mActions.append(mActionShop);
    connect(mActionShop, &QAction::triggered, this, &ComicApplet::slotShop);

    mActionSaveComicAs = new QAction(QIcon::fromTheme(QStringLiteral("document-save-as")), i18nc("@action", "&Save Comic As…"), this);
    mActions.append(mActionSaveComicAs);
    connect(mActionSaveComicAs, &QAction::triggered, this, &ComicApplet::slotSaveComicAs);

    mActionCreateComicBook = new QAction(QIcon::fromTheme(QStringLiteral("application-epub+zip")), i18nc("@action", "&Create Comic Book Archive…"), this);
    mActions.append(mActionCreateComicBook);
    connect(mActionCreateComicBook, &QAction::triggered, this, &ComicApplet::createComicBook);

    mActionScaleContent = new QAction(QIcon::fromTheme(QStringLiteral("zoom-original")), //
                                      i18nc("@option:check Context menu of comic image", "&Actual Size"),
                                      this);
    mActionScaleContent->setCheckable(true);
    mActionScaleContent->setChecked(mCurrent.scaleComic());
    mActions.append(mActionScaleContent);
    connect(mActionScaleContent, &QAction::triggered, this, &ComicApplet::slotScaleToContent);

    mActionStorePosition =
        new QAction(QIcon::fromTheme(QStringLiteral("go-home")), i18nc("@option:check Context menu of comic image", "Store Current &Position"), this);
    mActionStorePosition->setCheckable(true);
    mActionStorePosition->setChecked(mCurrent.hasStored());
    mActions.append(mActionStorePosition);
    connect(mActionStorePosition, &QAction::triggered, this, &ComicApplet::slotStorePosition);

    // make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    updateUsedComics();
    changeComic(true);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    connect(&m_networkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged, this, [this](bool isOnline) {
        if (!isOnline) {
            return;
        }
        qCDebug(PLASMA_COMIC) << "Online status changed to true, requesting comic" << mPreviousFailedIdentifier;
        mEngine->requestSource(mPreviousFailedIdentifier);
    });
    QT_WARNING_POP
}

ComicApplet::~ComicApplet()
{
    delete mSavingDir;
}

void ComicApplet::dataUpdated(const ComicMetaData &data)
{
    const QString source = data.identifier;
    if (source.startsWith(mOldSource)) {
        mOldSource = source;
    }
    setBusy(false);

    // disconnect prefetched comic strips
    if (source != mOldSource) {
        return;
    }

    setConfigurationRequired(false);

    // there was an error, display information as image
    if (data.error) {
        mPreviousFailedIdentifier = source;
        if (mEngine && !mShowErrorPicture && !data.previousIdentifier.isEmpty()) {
            updateComic(data.previousIdentifier);
        }
        return;
    }

    mCurrent.setData(data);

    setAssociatedApplicationUrls(QList<QUrl>() << mCurrent.websiteUrl());

    // looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if (!mCurrent.hasNext() && mCheckNewComicStripsInterval) {
        setTabHighlighted(mCurrent.id(), false);
        mActionNextNewStripTab->setEnabled(isTabHighlighted(mCurrent.id()));
    }

    // call the slot to check if the position needs to be saved
    slotStorePosition();

    if (mEngine) {
        // prefetch the previous and following comic for faster navigation
        if (mCurrent.hasNext()) {
            const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.next();
            mEngine->requestSource(prefetch);
        }
        if (mCurrent.hasPrev()) {
            const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.prev();
            mEngine->requestSource(prefetch);
        }
    }

    updateView();

    refreshComicData();
}

void ComicApplet::updateView()
{
    updateContextMenu();
}

void ComicApplet::positionFullView(QWindow *window)
{
    if (!window || !window->screen()) {
        return;
    }

    window->setPosition(window->screen()->availableGeometry().center() - QPoint(window->size().width() / 2, window->size().height() / 2));
}

void ComicApplet::changeComic(bool differentComic)
{
    if (differentComic) {
        KConfigGroup cg = config();
        mActionStorePosition->setChecked(mCurrent.storePosition());

        // assign mScaleComic the moment the new strip has been loaded (dataUpdated) as up to this point
        // the old one should be still shown with its scaling settings
        mActionScaleContent->setChecked(mCurrent.scaleComic());

        updateComic(mCurrent.stored());
    } else {
        updateComic(mCurrent.current());
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
    for (int i = 0; i < mProxy->rowCount(); ++i) {
        if (mTabIdentifier.contains(mProxy->index(i, 0).data(Qt::UserRole).toString())) {
            data = mProxy->index(i, 1);

            if (isFirst) {
                isFirst = false;
                const QString id = data.data(Qt::UserRole).toString();
                mDifferentComic = (oldIdentifier != id);
                const QString title = data.data().toString();
                mCurrent.init(id, config());
                mCurrent.setTitle(title);
            }

            const QString name = data.data().toString();
            const QString identifier = data.data(Qt::UserRole).toString();
            const QIcon icon = data.data(Qt::DecorationRole).value<QIcon>();
            // found a newer strip last time, which was not visited

            if (mCheckNewComicStripsInterval && !cg.readEntry(QLatin1String("lastStripVisited_") + identifier, true)) {
                mActiveComicModel->addComic(identifier, name, icon, true);
            } else {
                mActiveComicModel->addComic(identifier, name, icon);
            }

            ++tab;
        }
    }

    mActionNextNewStripTab->setVisible(mCheckNewComicStripsInterval);
    mActionNextNewStripTab->setEnabled(isTabHighlighted(mCurrent.id()));

    delete mCheckNewStrips;
    mCheckNewStrips = nullptr;
    if (mEngine && mCheckNewComicStripsInterval) {
        mCheckNewStrips = new CheckNewStrips(mTabIdentifier, mEngine, mCheckNewComicStripsInterval, this);
        connect(mCheckNewStrips, &CheckNewStrips::lastStrip, this, &ComicApplet::slotFoundLastStrip);
    }

    Q_EMIT comicModelChanged();
}

void ComicApplet::slotTabChanged(const QString &identifier)
{
    bool differentComic = (mCurrent.id() != identifier);
    mCurrent = ComicData();
    mCurrent.init(identifier, config());
    changeComic(differentComic);
}

void ComicApplet::checkDayChanged()
{
    if (mCurrentDay != QDate::currentDate()) {
        updateComic(mCurrent.current());
        mCurrentDay = QDate::currentDate();
    } else if (!mCurrent.hasImage()) {
        updateComic(mCurrent.stored());
    }
}

void ComicApplet::configChanged()
{
    KConfigGroup cg = config();
    mTabIdentifier = cg.readEntry("tabIdentifier", QStringList());

    if (mProxy) {
        updateUsedComics();
    }

    const QString id = mTabIdentifier.count() ? mTabIdentifier.at(0) : QString();
    mCurrent = ComicData();
    mCurrent.init(id, cg);

    mShowComicUrl = cg.readEntry("showComicUrl", false);
    mShowComicAuthor = cg.readEntry("showComicAuthor", false);
    mShowComicTitle = cg.readEntry("showComicTitle", false);
    mShowComicIdentifier = cg.readEntry("showComicIdentifier", false);
    mShowErrorPicture = cg.readEntry("showErrorPicture", true);
    mArrowsOnHover = cg.readEntry("arrowsOnHover", true);
    mMiddleClick = cg.readEntry("middleClick", true);
    mCheckNewComicStripsInterval = cg.readEntry("checkNewComicStripsIntervall", 30);

    auto oldMaxComicLimit = mMaxComicLimit;
    mMaxComicLimit = cg.readEntry("maxComicLimit", 29);
    if (oldMaxComicLimit != mMaxComicLimit && mEngine) {
        mEngine->setMaxComicLimit(mMaxComicLimit);
    }

    globalComicUpdater->load();
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry("comic", mCurrent.id());
    cg.writeEntry("showComicUrl", mShowComicUrl);
    cg.writeEntry("showComicAuthor", mShowComicAuthor);
    cg.writeEntry("showComicTitle", mShowComicTitle);
    cg.writeEntry("showComicIdentifier", mShowComicIdentifier);
    cg.writeEntry("showErrorPicture", mShowErrorPicture);
    cg.writeEntry("arrowsOnHover", mArrowsOnHover);
    cg.writeEntry("middleClick", mMiddleClick);
    cg.writeEntry("tabIdentifier", mTabIdentifier);
    cg.writeEntry("checkNewComicStripsIntervall", mCheckNewComicStripsInterval);
    cg.writeEntry("maxComicLimit", mMaxComicLimit);

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

void ComicApplet::slotFoundLastStrip(int index, const QString &identifier, const QString &suffix)
{
    Q_UNUSED(index)

    if (mCurrent.id() != identifier) {
        return;
    }

    KConfigGroup cg = config();
    if (suffix != cg.readEntry(QLatin1String("lastStrip_") + identifier, QString())) {
        qDebug() << identifier << "has a newer strip.";
        cg.writeEntry(QLatin1String("lastStripVisited_") + identifier, false);
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
    auto *job = new KIO::OpenUrlJob(mCurrent.shopUrl());
    job->start();
}

void ComicApplet::createComicBook()
{
    ComicArchiveDialog *dialog = new ComicArchiveDialog(mCurrent.id(), //
                                                        mCurrent.title(),
                                                        mCurrent.type(),
                                                        mCurrent.current(),
                                                        mCurrent.first(),
                                                        mSavingDir->getDir());
    dialog->setAttribute(Qt::WA_DeleteOnClose); // to have destroyed emitted upon closing
    connect(dialog, &ComicArchiveDialog::archive, this, &ComicApplet::slotArchive);
    dialog->show();
}

void ComicApplet::slotArchive(int archiveType, const QUrl &dest, const QString &fromIdentifier, const QString &toIdentifier)
{
    if (!mEngine) {
        return;
    }

    mSavingDir->setDir(dest.path());

    const QString id = mCurrent.id();
    qDebug() << "Archiving:" << id << archiveType << dest << fromIdentifier << toIdentifier;
    ComicArchiveJob *job = new ComicArchiveJob(dest, mEngine, static_cast<ComicArchiveJob::ArchiveType>(archiveType), mCurrent.type(), id, this);
    job->setFromIdentifier(id + QLatin1Char(':') + fromIdentifier);
    job->setToIdentifier(id + QLatin1Char(':') + toIdentifier);
    if (job->isValid()) {
        connect(job, &ComicArchiveJob::finished, this, &ComicApplet::slotArchiveFinished);
        KIO::getJobTracker()->registerJob(job);
        job->start();
    } else {
        qWarning() << "Archiving job is not valid.";
        delete job;
    }
}

void ComicApplet::slotArchiveFinished(KJob *job)
{
    if (job->error()) {
        KNotification::event(KNotification::Warning, i18n("Archiving comic failed"), job->errorText(), QStringLiteral("dialog-warning"));
    }
}

QList<QAction *> ComicApplet::contextualActions()
{
    return mActions;
}

void ComicApplet::updateComic(const QString &identifierSuffix)
{
    const QString id = mCurrent.id();
    setConfigurationRequired(id.isEmpty());

    if (!id.isEmpty() && mEngine) {
        setBusy(true);

        const QString identifier = id + QLatin1Char(':') + identifierSuffix;

        // disconnecting of the oldSource is needed, otherwise you could get data for comics you are not looking at if you use tabs
        // if there was an error only disconnect the oldSource if it had nothing to do with the error or if the comic changed, that way updates of the error can
        // come in
        if (!mIdentifierError.isEmpty() && !mIdentifierError.contains(id)) {
            mIdentifierError.clear();
        }
        mOldSource = identifier;
        mEngine->requestSource(identifier);
        slotScaleToContent();
    } else {
        qWarning() << "Either no identifier was specified or the engine could not be created:"
                   << "id" << id;
        setConfigurationRequired(true);
    }
    updateContextMenu();
}

void ComicApplet::updateContextMenu()
{
    if (mCurrent.id().isEmpty()) {
        mActiveComicModel->clear();
        mActionNextNewStripTab->setEnabled(false);
        mActionGoFirst->setEnabled(false);
        mActionGoLast->setEnabled(false);
        mActionScaleContent->setEnabled(false);
        if (mActionShop) {
            mActionShop->setEnabled(false);
        }
        mActionStorePosition->setEnabled(false);
        mActionGoJump->setEnabled(false);
        mActionSaveComicAs->setEnabled(false);
        mActionCreateComicBook->setEnabled(false);
        mActionScaleContent->setChecked(false);
    } else {
        mActionGoFirst->setVisible(mCurrent.hasFirst());
        mActionGoFirst->setEnabled(mCurrent.hasPrev());
        mActionGoLast->setEnabled(true);
        if (mActionShop) {
            mActionShop->setEnabled(mCurrent.shopUrl().isValid());
        }
        mActionScaleContent->setEnabled(true);
        mActionStorePosition->setEnabled(true);
        mActionGoJump->setEnabled(true);
        mActionSaveComicAs->setEnabled(true);
        mActionCreateComicBook->setEnabled(true);
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

// QML
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
    if (show == mShowComicUrl) {
        return;
    }

    mShowComicUrl = show;

    Q_EMIT showComicUrlChanged();
}

bool ComicApplet::showComicAuthor() const
{
    return mShowComicAuthor;
}

void ComicApplet::setShowComicAuthor(bool show)
{
    if (show == mShowComicAuthor) {
        return;
    }

    mShowComicAuthor = show;

    Q_EMIT showComicAuthorChanged();
}

bool ComicApplet::showComicTitle() const
{
    return mShowComicTitle;
}

void ComicApplet::setShowComicTitle(bool show)
{
    if (show == mShowComicTitle) {
        return;
    }

    mShowComicTitle = show;

    Q_EMIT showComicTitleChanged();
}

bool ComicApplet::showComicIdentifier() const
{
    return mShowComicIdentifier;
}

void ComicApplet::setShowComicIdentifier(bool show)
{
    if (show == mShowComicIdentifier) {
        return;
    }

    mShowComicIdentifier = show;

    Q_EMIT showComicIdentifierChanged();
}

bool ComicApplet::showErrorPicture() const
{
    return mShowErrorPicture;
}

void ComicApplet::setShowErrorPicture(bool show)
{
    if (show == mShowErrorPicture) {
        return;
    }

    mShowErrorPicture = show;

    Q_EMIT showErrorPictureChanged();
}

bool ComicApplet::arrowsOnHover() const
{
    return mArrowsOnHover;
}

void ComicApplet::setArrowsOnHover(bool show)
{
    if (show == mArrowsOnHover) {
        return;
    }

    mArrowsOnHover = show;

    Q_EMIT arrowsOnHoverChanged();
}

bool ComicApplet::middleClick() const
{
    return mMiddleClick;
}

void ComicApplet::setMiddleClick(bool show)
{
    if (show == mMiddleClick) {
        return;
    }

    mMiddleClick = show;

    Q_EMIT middleClickChanged();
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
    Q_EMIT tabIdentifiersChanged();
    saveConfig();
    changeComic(mDifferentComic);
}

void ComicApplet::refreshComicData()
{
    mComicData[QStringLiteral("image")] = mCurrent.image();
    mComicData[QStringLiteral("prev")] = mCurrent.prev();
    mComicData[QStringLiteral("next")] = mCurrent.next();
    mComicData[QStringLiteral("additionalText")] = mCurrent.additionalText();

    mComicData[QStringLiteral("websiteUrl")] = mCurrent.websiteUrl().toString();
    mComicData[QStringLiteral("websiteHost")] = mCurrent.websiteUrl().host();
    mComicData[QStringLiteral("imageUrl")] = mCurrent.websiteUrl().toString();
    mComicData[QStringLiteral("shopUrl")] = mCurrent.websiteUrl().toString();
    mComicData[QStringLiteral("first")] = mCurrent.first();
    mComicData[QStringLiteral("stripTitle")] = mCurrent.stripTitle();
    mComicData[QStringLiteral("author")] = mCurrent.author();
    mComicData[QStringLiteral("title")] = mCurrent.title();

    mComicData[QStringLiteral("suffixType")] = QStringLiteral("Date");
    mComicData[QStringLiteral("current")] = mCurrent.current();
    // mComicData[QStringLiteral("last")] = mCurrent.last();
    mComicData[QStringLiteral("currentReadable")] = mCurrent.currentReadable();
    mComicData[QStringLiteral("firstStripNum")] = mCurrent.firstStripNum();
    mComicData[QStringLiteral("maxStripNum")] = mCurrent.maxStripNum();
    mComicData[QStringLiteral("isLeftToRight")] = mCurrent.isLeftToRight();
    mComicData[QStringLiteral("isTopToBottom")] = mCurrent.isTopToBottom();

    Q_EMIT comicDataChanged();
}

bool ComicApplet::showActualSize() const
{
    return mCurrent.scaleComic();
}

void ComicApplet::setShowActualSize(bool show)
{
    if (show == mCurrent.scaleComic()) {
        return;
    }

    mCurrent.setScaleComic(show);

    Q_EMIT showActualSizeChanged();
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
    Q_EMIT checkNewComicStripsIntervalChanged();
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
    Q_EMIT providerUpdateIntervalChanged();
}

void ComicApplet::setMaxComicLimit(int limit)
{
    if (mMaxComicLimit == limit) {
        return;
    }

    mMaxComicLimit = limit;
    Q_EMIT maxComicLimitChanged();
}

int ComicApplet::maxComicLimit() const
{
    return mMaxComicLimit;
}

// Endof QML
void ComicApplet::setTabHighlighted(const QString &id, bool highlight)
{
    // Search for matching id
    for (int i = 0; i < mActiveComicModel->rowCount(); ++i) {
        QStandardItem *item = mActiveComicModel->item(i);

        QString currentId = item->data(ActiveComicModel::ComicKeyRole).toString();
        if (id == currentId) {
            if (highlight != item->data(ActiveComicModel::ComicHighlightRole).toBool()) {
                item->setData(highlight, ActiveComicModel::ComicHighlightRole);
                Q_EMIT tabHighlightRequest(id, highlight);
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

K_PLUGIN_CLASS(ComicApplet)

#include "comic.moc"
