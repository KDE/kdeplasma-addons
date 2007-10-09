/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef EBNENGINE_H
#define EBNENGINE_H

#include <syndication/loader.h>
#include <syndication/feed.h>
#include <QtCore/QHash>

#include "plasma/dataengine.h"

/**
 * Processes feeds from the English Breakfast Network
 *
 * There are three keys returned:
 *  - title: the title of the feed
 *  - link:  the link the html version of the feed
 *  - items: a QVariantMap associating the details of
 *           the module, component or directory with its title
 *
 * The details are stored in a QVariantMap like so:
 *  - issues: the number of issues (as a QString)
 *  - link:   the link to the html version of the list
 *  - source: (only if the item is a module or component)
 *            a source that can be requested from the engine
 *
 * NB: Plasmaengineexplorer currently can't cope with
 *     QVariantMap.
 */
class EbnEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        EbnEngine( QObject* parent, const QVariantList& args );
        ~EbnEngine();

    protected:
        void init();
        bool sourceRequested(const QString &name);

    protected slots:
        bool updateSource(const QString &name);
        void processFeed(Syndication::Loader* loader,
                         Syndication::FeedPtr feed,
                         Syndication::ErrorCode error);
};

K_EXPORT_PLASMA_DATAENGINE(ebn, EbnEngine)

#endif // EBNENGINE_H
