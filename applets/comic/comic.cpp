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
#include "stripselector.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QNetworkInformation>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QWindow>

#include <KIO/OpenUrlJob>
#include <KLocalizedString>

#include "comicmodel.h"

ComicApplet::ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
    , mCheckNewStrips(nullptr)
    , mEngine(new ComicEngine(this))
{
    setHasConfigurationInterface(true);
}

void ComicApplet::init()
{
    mDateChangedTimer = new QTimer(this);
    mDateChangedTimer->setInterval(5 * 60 * 1000); // every 5 minutes

    configChanged();

    KConfigGroup cg = config();
    QStringList tabIdentifier = cg.readEntry("tabIdentifier", QStringList());

    mModel = new ComicModel(mEngine, tabIdentifier, this);

    mCurrentDay = QDate::currentDate();
    connect(mDateChangedTimer, &QTimer::timeout, this, &ComicApplet::checkDayChanged);

    updateUsedComics();

    if (!tabIdentifier.isEmpty()) {
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
    if (!data.error && !mCurrent.hasNext() && cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 0)) {
        setTabHighlighted(mCurrent.id(), false);
    }

    storePosition(mCurrent.hasStored());

    // prefetch the previous and following comic for faster navigation
    if (!data.error && mCurrent.hasNext()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.next();
        mEngine->requestSource(prefetch);
    }
    if (!data.error && mCurrent.hasPrev()) {
        const QString prefetch = mCurrent.id() + QLatin1Char(':') + mCurrent.prev();
        mEngine->requestSource(prefetch);
    }

    refreshComicData();
    Q_EMIT showActualSizeChanged(); // if switching comics the new one might have a different setting
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
    loadProviders();

    KConfigGroup cg = config();
    for (int i = 0; i < mModel->rowCount(); ++i) {
        QModelIndex index = mModel->index(i, 0);
        if (index.data(ComicModel::Roles::ComicEnabledRole).toBool() && cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 0)
            && !cg.readEntry(QLatin1String("lastStripVisited_") + index.data(ComicModel::Roles::ComicPluginRole).toString(), true)) {
            mModel->setHighlight(index, true);
        }
    }

    delete mCheckNewStrips;
    mCheckNewStrips = nullptr;
    if (cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 0)) {
        mCheckNewStrips =
            new CheckNewStrips(cg.readEntry("tabIdentifier", QStringList()), mEngine, cg.readEntry(QLatin1String("checkNewComicStripsIntervall"), 0), this);
        connect(mCheckNewStrips, &CheckNewStrips::lastStrip, this, &ComicApplet::slotFoundLastStrip);
    }
}

void ComicApplet::tabChanged(const QString &identifier)
{
    if (mCurrent.id() == identifier) {
        return;
    }
    mCurrent = ComicData();
    mCurrent.init(identifier, config());

    KConfigGroup cg = config();
    if (!cg.readEntry("tabIdentifier", QStringList()).isEmpty()) {
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

    QStringList tabIdentifier = cg.readEntry("tabIdentifier", QStringList());

    if (mModel) {
        updateUsedComics();
    }

    const QString id = tabIdentifier.count() ? tabIdentifier.at(0) : QString();
    mCurrent = ComicData();
    mCurrent.init(id, cg);

    mEngine->setMaxComicLimit(cg.readEntry("maxComicLimit", 29));
    if (tabIdentifier.isEmpty()) {
        mDateChangedTimer->stop();
    } else {
        mDateChangedTimer->start();
    }
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry("comic", mCurrent.id());
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

void ComicApplet::goJump()
{
    StripSelector *selector = StripSelectorFactory::create(mCurrent.type());
    connect(selector, &StripSelector::stripChosen, this, &ComicApplet::updateComic);

    selector->select(mCurrent);
}

void ComicApplet::storePosition(bool store)
{
    mCurrent.storePosition(store);
}

void ComicApplet::website()
{
    auto *job = new KIO::OpenUrlJob(mCurrent.websiteUrl());
    job->start();
}

void ComicApplet::shop()
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
}

bool ComicApplet::saveImage(const QUrl &fileUrl)
{
    if (!fileUrl.isValid()) {
        return false;
    }

    return mCurrent.image().save(fileUrl.toLocalFile(), "PNG");
}

void ComicApplet::slotShowActualSize(bool scale)
{
    setShowActualSize(scale);
}

// QML

QObject *ComicApplet::availableComicsModel() const
{
    return mModel;
}

QVariantMap ComicApplet::comicData() const
{
    return mComicData;
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

// Endof QML
void ComicApplet::setTabHighlighted(const QString &id, bool highlight)
{
    QModelIndexList indexList = mModel->match(mModel->index(0, 0), ComicModel::ComicPluginRole, id, 1, Qt::MatchFixedString);
    if (!indexList.empty()) {
        mModel->setHighlight(indexList[0], highlight);
    }
}

void ComicApplet::loadProviders()
{
    mModel->setEnabledProviders(config().readEntry("tabIdentifier", QStringList()));
}

bool ComicApplet::urlExists(const QUrl &url)
{
    QDir dir(url.path());
    return dir.exists();
}

K_PLUGIN_CLASS_WITH_JSON(ComicApplet, "metadata.json")

#include "comic.moc"
