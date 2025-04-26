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
#include "comicsaver.h"
#include "stripselector.h"

#include <QAction>
#include <QDebug>
#include <QNetworkInformation>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QWindow>

#include <KActionCollection>
#include <KApplicationTrader>
#include <KConfigDialog>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KNotification>
#include <KService>
#include <KStandardShortcut>
#include <kuiserverjobtracker.h>

#include <Plasma/Containment>

#include "comicmodel.h"

ComicApplet::ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
    , mProxy(nullptr)
    , mActiveComicModel(new ActiveComicModel(parent))
    , mShowComicUrl(false)
    , mShowComicAuthor(false)
    , mShowComicTitle(false)
    , mShowComicIdentifier(false)
    , mArrowsOnHover(true)
    , mMiddleClick(true)
    , mCheckNewComicStripsInterval(0)
    , mMaxComicLimit(0)
    , mCheckNewStrips(nullptr)
    , mEngine(new ComicEngine(this))
    , mSavingDir(nullptr)
{
    setHasConfigurationInterface(true);
}

void ComicApplet::init()
{
    mSavingDir = new SavingDir(config());

    mDateChangedTimer = new QTimer(this);
    mDateChangedTimer->setInterval(5 * 60 * 1000); // every 5 minutes

    configChanged();

    mModel = new ComicModel(mEngine, mTabIdentifier, this);
    mProxy = new QSortFilterProxyModel(this);
    mProxy->setSourceModel(mModel);
    mProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxy->sort(1, Qt::AscendingOrder);

    mCurrentDay = QDate::currentDate();
    connect(mDateChangedTimer, &QTimer::timeout, this, &ComicApplet::checkDayChanged);

    // make sure that tabs etc. are displayed even if the comic strip in the first tab does not work
    updateView();

    updateUsedComics();
    if (!mTabIdentifier.isEmpty()) {
        updateComic(mCurrent.stored());
    }

    connect(mEngine, &ComicEngine::requestFinished, this, &ComicApplet::dataUpdated);

    QNetworkInformation::instance()->loadBackendByFeatures(QNetworkInformation::Feature::Reachability);
    connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, this, [this](auto reachability) {
        if (reachability != QNetworkInformation::Reachability::Online) {
            return;
        }
        if (!mPreviousFailedIdentifier.isEmpty()) {
            qCDebug(PLASMA_COMIC) << "Online status changed to true, requesting comic" << mPreviousFailedIdentifier;
            mEngine->requestSource(mPreviousFailedIdentifier);
        }
    });
}

ComicApplet::~ComicApplet()
{
    delete mSavingDir;
}

void ComicApplet::dataUpdated(const ComicMetaData &data)
{
    const QString source = data.identifier;
    setBusy(false);

    // disconnect prefetched comic strips
    if (source != mOldSource) {
        return;
    }

    // there was an error, display information as image
    if (data.error) {
        mPreviousFailedIdentifier = source;
    } else {
        mPreviousFailedIdentifier.clear();
    }

    mCurrent.setData(data);

    // looking at the last index, thus not mark it as new
    KConfigGroup cg = config();
    if (!data.error && !mCurrent.hasNext() && mCheckNewComicStripsInterval) {
        setTabHighlighted(mCurrent.id(), false);
    }

    // call the slot to check if the position needs to be saved
    slotStorePosition(mCurrent.hasStored());

    // prefetch the previous and following comic for faster navigation
    if (!data.error && mCurrent.hasNext()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.next();
        mEngine->requestSource(prefetch);
    }
    if (!data.error && mCurrent.hasPrev()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.prev();
        mEngine->requestSource(prefetch);
    }

    updateView();

    refreshComicData();
    Q_EMIT showActualSizeChanged(); // if switching comics the new one might have a different setting
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

void ComicApplet::updateUsedComics()
{
    mActiveComicModel->clear();
    mCurrent = ComicData();

    bool isFirst = true;
    QModelIndex data;
    KConfigGroup cg = config();
    for (int i = 0; i < mProxy->rowCount(); ++i) {
        if (mTabIdentifier.contains(mProxy->index(i, 0).data(Qt::UserRole).toString())) {
            data = mProxy->index(i, 1);

            if (isFirst) {
                isFirst = false;
                const QString id = data.data(Qt::UserRole).toString();
                const QString title = data.data().toString();
                mCurrent.init(id, config());
                mCurrent.setTitle(title);
            }

            const QString name = data.data().toString();
            const QString identifier = data.data(Qt::UserRole).toString();
            const QString icon = data.data(Qt::DecorationRole).toString();
            // found a newer strip last time, which was not visited

            if (mCheckNewComicStripsInterval && !cg.readEntry(QLatin1String("lastStripVisited_") + identifier, true)) {
                mActiveComicModel->addComic(identifier, name, icon, true);
            } else {
                mActiveComicModel->addComic(identifier, name, icon);
            }
        }
    }

    delete mCheckNewStrips;
    mCheckNewStrips = nullptr;
    if (mCheckNewComicStripsInterval) {
        mCheckNewStrips = new CheckNewStrips(mTabIdentifier, mEngine, mCheckNewComicStripsInterval, this);
        connect(mCheckNewStrips, &CheckNewStrips::lastStrip, this, &ComicApplet::slotFoundLastStrip);
    }

    Q_EMIT comicModelChanged();
}

void ComicApplet::slotTabChanged(const QString &identifier)
{
    if (mCurrent.id() == identifier) {
        return;
    }
    mCurrent = ComicData();
    mCurrent.init(identifier, config());
    if (!mTabIdentifier.isEmpty()) {
        updateComic(mCurrent.stored());
    }
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
    mArrowsOnHover = cg.readEntry("arrowsOnHover", true);
    mMiddleClick = cg.readEntry("middleClick", true);
    mCheckNewComicStripsInterval = cg.readEntry("checkNewComicStripsIntervall", 30);

    auto oldMaxComicLimit = mMaxComicLimit;
    mMaxComicLimit = cg.readEntry("maxComicLimit", 29);
    if (oldMaxComicLimit != mMaxComicLimit) {
        mEngine->setMaxComicLimit(mMaxComicLimit);
    }
    if (mTabIdentifier.isEmpty()) {
        mDateChangedTimer->stop();
    } else {
        mDateChangedTimer->start();
    }
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry("comic", mCurrent.id());
    cg.writeEntry("showComicUrl", mShowComicUrl);
    cg.writeEntry("showComicAuthor", mShowComicAuthor);
    cg.writeEntry("showComicTitle", mShowComicTitle);
    cg.writeEntry("showComicIdentifier", mShowComicIdentifier);
    cg.writeEntry("arrowsOnHover", mArrowsOnHover);
    cg.writeEntry("middleClick", mMiddleClick);
    cg.writeEntry("tabIdentifier", mTabIdentifier);
    cg.writeEntry("checkNewComicStripsIntervall", mCheckNewComicStripsInterval);
    cg.writeEntry("maxComicLimit", mMaxComicLimit);
}

void ComicApplet::slotNextDay()
{
    updateComic(mCurrent.next());
}

void ComicApplet::slotPreviousDay()
{
    updateComic(mCurrent.prev());
}

void ComicApplet::slotFoundLastStrip(int index, const QString &identifier, const QString &suffix)
{
    Q_UNUSED(index)

    if (mCurrent.id() != identifier) {
        return;
    }

    KConfigGroup cg = config();
    if (suffix != cg.readEntry(QLatin1String("lastStrip_") + identifier, QString())) {
        qCDebug(PLASMA_COMIC) << identifier << "has a newer strip.";
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

void ComicApplet::slotStorePosition(bool store)
{
    mCurrent.storePosition(store);
}

void ComicApplet::slotWebsite()
{
    auto *job = new KIO::OpenUrlJob(mCurrent.websiteUrl());
    job->start();
}

void ComicApplet::slotShop()
{
    auto *job = new KIO::OpenUrlJob(mCurrent.shopUrl());
    job->start();
}

void ComicApplet::updateComic(const QString &identifierSuffix)
{
    const QString id = mCurrent.id();

    if (!id.isEmpty()) {
        setBusy(true);

        const QString identifier = id + QLatin1Char(':') + identifierSuffix;

        // disconnecting of the oldSource is needed, otherwise you could get data for comics you are not looking at if you use tabs
        // if there was an error only disconnect the oldSource if it had nothing to do with the error or if the comic changed, that way updates of the error can
        // come in
        mOldSource = identifier;
        mEngine->requestSource(identifier);
        slotShowActualSize(mCurrent.showActualSize());
    } else {
        setBusy(false);
        qCWarning(PLASMA_COMIC) << "Either no identifier was specified or the engine could not be created:"
                                << "id" << id;
    }
    updateContextMenu();
}

void ComicApplet::updateContextMenu()
{
    if (mCurrent.id().isEmpty()) {
        mActiveComicModel->clear();
    }
}

void ComicApplet::slotSaveComicAs()
{
    ComicSaver saver(mSavingDir);
    saver.save(mCurrent);
}

void ComicApplet::slotShowActualSize(bool scale)
{
    setShowActualSize(scale);
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
    if (mTabIdentifier.contains(mCurrent.id())) {
        updateComic(mCurrent.current());
    } else {
        updateComic(mCurrent.stored());
    }
}

void ComicApplet::refreshComicData()
{
    mComicData[QStringLiteral("id")] = mCurrent.id();
    mComicData[QStringLiteral("image")] = mCurrent.image();
    mComicData[QStringLiteral("prev")] = mCurrent.prev();
    mComicData[QStringLiteral("next")] = mCurrent.next();
    mComicData[QStringLiteral("additionalText")] = mCurrent.additionalText();

    mComicData[QStringLiteral("websiteUrl")] = mCurrent.websiteUrl().toString();
    mComicData[QStringLiteral("websiteHost")] = mCurrent.websiteUrl().host();
    mComicData[QStringLiteral("imageUrl")] = mCurrent.websiteUrl().toString();
    mComicData[QStringLiteral("shopUrl")] = mCurrent.shopUrl().toString();
    mComicData[QStringLiteral("first")] = mCurrent.first();
    mComicData[QStringLiteral("hasFirst")] = mCurrent.hasFirst();
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
    mComicData[QStringLiteral("isError")] = mCurrent.isError();
    mComicData[QStringLiteral("storePosition")] = mCurrent.storePosition();
    mComicData[QStringLiteral("ready")] = mCurrent.ready();
    mComicData[QStringLiteral("nextNewStripEnabled")] = isTabHighlighted(mCurrent.id());

    Q_EMIT comicDataChanged();
}

bool ComicApplet::showActualSize() const
{
    return mCurrent.showActualSize();
}

void ComicApplet::setShowActualSize(bool show)
{
    if (show == mCurrent.showActualSize()) {
        return;
    }

    mCurrent.setShowActualSize(show);

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

void ComicApplet::loadProviders()
{
    mModel->load();
}

K_PLUGIN_CLASS(ComicApplet)

#include "comic.moc"
