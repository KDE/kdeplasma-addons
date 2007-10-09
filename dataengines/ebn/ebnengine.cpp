/*
 *   Copyright (C) 2007 Alex Merry <huntedhacker@tiscali.co.uk>
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

#include "ebnengine.h"

#include <QDateTime>
#include <QTimer>

#include <KDebug>
#include <KLocale>
#include <KUrl>

#include <syndication/item.h>

EbnEngine::EbnEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)

    setMinimumUpdateInterval(60 * 1000);
    setUpdateInterval(60 * 60 * 1000);
}

EbnEngine::~EbnEngine()
{
}

void EbnEngine::init()
{
}

bool EbnEngine::updateSource(const QString &name)
{
    QString section = name.section('/', 0, 0, QString::SectionSkipEmpty);
    QString component = name.section('/', 1, 1, QString::SectionSkipEmpty);
    QString module = name.section('/', 2, 2, QString::SectionSkipEmpty);
    QString url = QString("http://www.englishbreakfastnetwork.org/"
                          "%1/rssfeed.php?component=%2&module=%3"
                          ).arg(section).arg(component).arg(module);

    // efficiency: questionable.  But Loader forces us to use it like this.
    Syndication::Loader* loader = Syndication::Loader::create();
    connect(loader, SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this, SLOT(processFeed(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(url);

    return false;
}

bool EbnEngine::sourceRequested(const QString &name)
{
    updateSource(name);

    return true;
}

/**
 * Takes an url of the form "http://www.englishbreakfastnetwork.org/\<section\>/index.php?component=\<component\>\&module=\<module\>"
 * and converts it to the form "\<section\>/\<component\>/\<module\>"
 */
static QString urlToSource(const QString& url)
{
    QString ret = url.section('/', 2, 2, QString::SectionSkipEmpty);
    QString work = url.section('=', 1, -1);
    if ( work.at(0) != '&' ) {
        ret += '/' + work.section('&', 0, 0);
        work = work.section('=', 1, 1);
        if ( ! work.isEmpty() ) {
            ret += '/' + work;
        }
    }
    return ret;
}

void EbnEngine::processFeed(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode error)
{
    Q_UNUSED(loader)
    if (error != Syndication::Success) {
        kDebug() << "syndication failed";
        //TODO: should probably tell the user it failed? =)
        //      note that if you give an incorrect module or component
        //      name, it will simply be ignored.  Only an incorrect
        //      section name (or a broken internet connection) will
        //      cause a failure
        return;
    }
    QString source = urlToSource(feed->link());
    setData(source, "title", feed->title());
    setData(source, "link", feed->link());

    kDebug() << "received " << feed->items().count() << " item(s) for " << feed->title();
    QVariantMap items;
    foreach (const Syndication::ItemPtr& item, feed->items()) {
        /*
        QStringList data;
        data << item->title().section(" - ", 1, 1).section(' ', 0, 0);
        data << item->link();
        if ( item->link().contains('?') ) {
            data << urlToSource(item->link());
        }
        */
        QVariantMap data;
        data["issues"] = item->title().section(" - ", 1, 1).section(' ', 0, 0);
        data["link"] = item->link();
        if ( item->link().contains('?') ) {
            data["source"] = urlToSource(item->link());
        }
        items[item->title().section(" - ", 0, 0)] = data;
    }
    setData(source, "items", items);
}


#include "ebnengine.moc"
