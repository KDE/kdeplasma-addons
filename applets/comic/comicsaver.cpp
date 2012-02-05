/***************************************************************************
 *   Copyright (C) 2008-2012 Matthias Fuchs <mat69@gmx.net>                *
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

#include "comicsaver.h"
#include "comicdata.h"
#include "comicinfo.h"

#include <KFileDialog>
#include <KIO/NetAccess>
#include <KTemporaryFile>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NFO>
#include <Nepomuk/Vocabulary/PIMO>
#include <nepomuk/utils.h>

using namespace Nepomuk::Vocabulary;
#endif

ComicSaver::ComicSaver(SavingDir *savingDir)
  : mSavingDir(savingDir)
{
#ifdef HAVE_NEPOMUK
    static bool isInit = false;
    if (!isInit) {
        isInit = true;
        // for manually saving the comics
        Nepomuk::ResourceManager::instance()->init();
    }
#endif
}

bool ComicSaver::save(const ComicData &comic)
{
    KTemporaryFile tempFile;

    if (!tempFile.open()) {
        return false;
    }

    // save image to temporary file
    comic.image().save(tempFile.fileName(), "PNG");

    KUrl srcUrl( tempFile.fileName() );

    const QString title = comic.title();

    const QString name = title + " - " + comic.current() + ".png";
    KUrl destUrl = KUrl(mSavingDir->getDir());
    destUrl.addPath( name );

    destUrl = KFileDialog::getSaveUrl( destUrl, "*.png" );
    if ( !destUrl.isValid() ) {
        return false;
    }

   mSavingDir->setDir(destUrl.directory());

#ifdef HAVE_NEPOMUK
    bool worked = KIO::NetAccess::file_copy(srcUrl, destUrl);
    //store additional data using Nepomuk
    if (worked) {
        Nepomuk::Resource res(destUrl, NFO::FileDataObject());

        Nepomuk::Resource comicTopic("Comic", PIMO::Topic());
        comicTopic.setLabel(i18n("Comic"));

        if (!comic.additionalText().isEmpty()) {
            res.setProperty(NIE::description(), comic.additionalText());
        }
        if ((comic.type() == Date) && !comic.current().isEmpty()) {
            res.setProperty(NIE::contentCreated(), QDateTime::fromString(comic.current(), Qt::ISODate));
        }
        if (!title.isEmpty()) {
            Nepomuk::Resource topic(title, PIMO::Topic());
            topic.setLabel(title);
            topic.setProperty(PIMO::superTopic(), comicTopic);
            res.addTag(topic);
        } else {
//             res.addTag(comicTopic);//TODO activate this, see below
            ;
        }

        //FIXME also set the comic topic as tag, this is redundant, as topic has this as super topic
        //though at this point the gui does not manage to show the correct tags
        res.addTag(comicTopic);

        if (!comic.stripTitle().isEmpty()) {
            res.setProperty(NIE::title(), comic.stripTitle());
        }
        if (!comic.websiteUrl().isEmpty()) {
            Nepomuk::Resource copyEvent = Nepomuk::Utils::createCopyEvent(comic.imageUrl(), destUrl, QDateTime(), comic.websiteUrl());
        }

        const QStringList authors = comic.author().split(',', QString::SkipEmptyParts);
        foreach (const QString &author, authors) {
            Nepomuk::Resource authorRes(author, NCO::PersonContact());
            authorRes.setProperty(NCO::fullname(), author.trimmed());
            res.addProperty(NCO::creator(), authorRes);
        }
        return true;
    }

    return false;
#else
    KIO::NetAccess::file_copy( srcUrl, destUrl );

    return true;
#endif
}
