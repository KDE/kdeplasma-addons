/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "comicprovider.h"

#include <QTimer>
#include <QUrl>
#include <QDebug>

#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KPluginMetaData>

class ComicProvider::Private
{
    public:
        Private(const KPluginMetaData &data, ComicProvider *parent)
            : mParent(parent),
              mIsCurrent(false),
              mFirstStripNumber(1),
              mComicDescription(data)
        {
            mTimer = new QTimer(parent);
            mTimer->setSingleShot(true);
            mTimer->setInterval(15000);//timeout after 15 seconds
            connect(mTimer, SIGNAL(timeout()), mParent, SLOT(slotTimeout()));
        }

        void jobDone(KJob *job)
        {
            if (job->error()) {
                mParent->pageError(job->property("uid").toInt(), job->errorText());
            } else {
                KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);
                mParent->pageRetrieved(job->property("uid").toInt(), storedJob->data());
            }
        }

        void slotRedirection(KIO::Job *job, QUrl newUrl)
        {
            slotRedirection(job, QUrl(), newUrl);
        }

        void slotRedirection(KIO::Job *job, QUrl oldUrl, QUrl newUrl)
        {
            Q_UNUSED(oldUrl)

            mParent->redirected(job->property("uid").toInt(), newUrl);
            mRedirections.remove(job);
        }

        void slotRedirectionDone(KJob *job)
        {
            if (job->error()) {
                qDebug() << "Redirection job with id" << job->property("uid").toInt() <<  "finished with an error.";
            }

            if (mRedirections.contains(job)) {
                //no redirection took place, return the original url
                mParent->redirected(job->property("uid").toInt(), mRedirections[job]);
                mRedirections.remove(job);
            }
        }

        void slotTimeout()
        {
            //operation took too long, abort it
            emit mParent->error(mParent);
        }

        void slotFinished()
        {
            //everything finished, stop the timeout timer
            mTimer->stop();
        }

        ComicProvider *mParent;
        QString mRequestedId;
        QString mRequestedComicName;
        QString mComicAuthor;
        QUrl mImageUrl;
        bool mIsCurrent;
        bool mIsLeftToRight;
        bool mIsTopToBottom;
        QDate mRequestedDate;
        QDate mFirstStripDate;
        int mRequestedNumber;
        int mFirstStripNumber;
        KPluginMetaData mComicDescription;
        QTimer *mTimer;
        QHash< KJob*, QUrl > mRedirections;
};

ComicProvider::ComicProvider(QObject *parent, const QVariantList &args)
    : QObject(parent), d(new Private(
      KPluginMetaData(args.count() > 2 ? args[2].toString() : QString()), this))
{
    Q_ASSERT(args.count() >= 2);
    const QString type = args[0].toString();

    if (type == QLatin1String("Date"))
        d->mRequestedDate = args[1].toDate();
    else if (type == QLatin1String("Number"))
        d->mRequestedNumber = args[1].toInt();
    else if (type == QLatin1String("String")) {
        d->mRequestedId = args[1].toString();

        int index = d->mRequestedId.indexOf(QLatin1Char(':'));
        d->mRequestedComicName = d->mRequestedId.mid(0, index);
    }
    else {
        Q_ASSERT(false && "Invalid type passed to comic provider");
    }

    d->mTimer->start();
    connect(this, SIGNAL(finished(ComicProvider*)), this, SLOT(slotFinished()));
}

ComicProvider::~ComicProvider()
{
    delete d;
}

QString ComicProvider::nextIdentifier() const
{
    if (identifierType() == DateIdentifier && d->mRequestedDate != QDate::currentDate())
        return d->mRequestedDate.addDays(1).toString(Qt::ISODate);

    return QString();
}

QString ComicProvider::previousIdentifier() const
{
    if ((identifierType() == DateIdentifier) && (!firstStripDate().isValid() || d->mRequestedDate > firstStripDate()))
        return d->mRequestedDate.addDays(-1).toString(Qt::ISODate);

    return QString();
}

QString ComicProvider::stripTitle() const
{
    return QString();
}

QString ComicProvider::additionalText() const
{
    return QString();
}

void ComicProvider::setIsCurrent(bool value)
{
    d->mIsCurrent = value;
}

bool ComicProvider::isCurrent() const
{
    return d->mIsCurrent;
}

QDate ComicProvider::requestedDate() const
{
    return d->mRequestedDate;
}

QDate ComicProvider::firstStripDate() const
{
    return d->mFirstStripDate;
}

QString ComicProvider::comicAuthor() const
{
    return d->mComicAuthor;
}

void ComicProvider::setComicAuthor(const QString &author)
{
    d->mComicAuthor = author;
}

void ComicProvider::setFirstStripDate(const QDate &date)
{
    d->mFirstStripDate = date;
}

int ComicProvider::firstStripNumber() const
{
    return d->mFirstStripNumber;
}

void ComicProvider::setFirstStripNumber(int number)
{
    d->mFirstStripNumber = number;
}

QString ComicProvider::firstStripIdentifier() const
{
    if ((identifierType() == DateIdentifier) && d->mFirstStripDate.isValid()) {
        return d->mFirstStripDate.toString(Qt::ISODate);
    } else if (identifierType() == NumberIdentifier) {
        return QString::number(d->mFirstStripNumber);
    }

    return QString();
}

int ComicProvider::requestedNumber() const
{
    return d->mRequestedNumber;
}

QString ComicProvider::requestedString() const
{
    return d->mRequestedId;
}

QString ComicProvider::requestedComicName() const
{
    return d->mRequestedComicName;
}

void ComicProvider::requestPage(const QUrl &url, int id, const MetaInfos &infos)
{
    //each request restarts the timer
    d->mTimer->start();

    if (id == Image) {
        d->mImageUrl = url;
    }

    KIO::StoredTransferJob *job;
    if (id == Image) {
        //use cached information for the image if available
        job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    } else {
        //for webpages we always reload, making sure, that changes are recognised
        job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    }
    job->setProperty("uid", id);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(jobDone(KJob*)));

    if (!infos.isEmpty()) {
        QMapIterator<QString, QString> it(infos);
        while (it.hasNext()) {
            it.next();
            job->addMetaData(it.key(), it.value());
        }
    }
}

void ComicProvider::requestRedirectedUrl(const QUrl &url, int id, const MetaInfos &infos)
{
    //each request restarts the timer
    d->mTimer->start();

    KIO::MimetypeJob *job = KIO::mimetype(url, KIO::HideProgressInfo);
    job->setProperty("uid", id);
    d->mRedirections[job] = url;
    connect(job, SIGNAL(redirection(KIO::Job*,QUrl)), this, SLOT(slotRedirection(KIO::Job*,QUrl)));
    connect(job, SIGNAL(permanentRedirection(KIO::Job*,QUrl,QUrl)), this, SLOT(slotRedirection(KIO::Job*,QUrl,QUrl)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRedirectionDone(KJob*)));

    if (!infos.isEmpty()) {
        QMapIterator<QString, QString> it(infos);
        while (it.hasNext()) {
            it.next();
            job->addMetaData(it.key(), it.value());
        }
    }
}

void ComicProvider::pageRetrieved(int, const QByteArray&)
{
}

void ComicProvider::pageError(int, const QString&)
{
}

void ComicProvider::redirected(int, const QUrl&)
{
}

QString ComicProvider::pluginName() const
{
    if (!d->mComicDescription.isValid()) {
        return QString();
    }
    return d->mComicDescription.pluginId();
}

QString ComicProvider::name() const
{
    if (!d->mComicDescription.isValid()) {
        return QString();
    }
    return d->mComicDescription.name();
}

QString ComicProvider::suffixType() const
{
    if (!d->mComicDescription.isValid()) {
        return QString();
    }
    return d->mComicDescription.value(QLatin1String("X-KDE-PlasmaComicProvider-SuffixType"));
}

KPluginMetaData ComicProvider::description() const
{
    return d->mComicDescription;
}

QUrl ComicProvider::shopUrl() const
{
    return QUrl();
}

QUrl ComicProvider::imageUrl() const
{
    return d->mImageUrl;
}

bool ComicProvider::isLeftToRight() const
{
    return true;
}

bool ComicProvider::isTopToBottom() const
{
    return true;
}

#include "moc_comicprovider.cpp"
