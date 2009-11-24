/*
 *   Copyright 2009 by Richard Moore <rich@kde.org>
 *   Copyright 2009 by Sebastian K?gler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef MEDIAWIKI_H
#define MEDIAWIKI_H

#include <QObject>
#include <QList>
#include <QUrl>

class QNetworkReply;

/**
 * Searches MediaWiki based wikis like wikipedia and techbase.
 *
 * @author Richard Moore, rich@kde.org
 */
class MediaWiki : public QObject
{
    Q_OBJECT

public:
    /**
     * Contains information about a single match from the search.
     */
    class Result {
        public:
            Result() {
        };

        Result( const Result &r ) {
            this->title = r.title;
            this->url = r.url;
        };

        Result &operator= (const Result &other)  {
            this->title = other.title;
            this->url = other.url;

            return *this;
        };

        /** The page title of the match. */
        QString title;
        /** The URL of the page containing the match. */
        QUrl url;
    };

    /**
     * Create a media wiki querying object with the specified parent. The querying
     * object can be used for multple queries, though only one can be performed at
     * a time.
     * @param parent The parent object
     */
    MediaWiki( QObject *parent=0 );
    virtual ~MediaWiki();

    /**
     * @returns a list of matches.
     */
    QList<MediaWiki::Result> results() const;

    /** @returns the currently specified maximum number of results to return. */
    int maxItems() const;

    /** Sets the maximum number of results to return.
     *
     * @param limit Maximumber number of results to retrieve
     */
    void setMaxItems( int limit );

    /** @returns the currently specified API URL. */
    QUrl apiUrl() const;

    /**
     * Sets the URL at which the wikis API page can be found. For example, wikipedia
     * has the API file at http://en.wikipedia.org/w/api.php whilst techbase has the
     *
     * @param url The URL of the api.php file, for example http://techbase.kde.org/api.php
     */
    void setApiUrl( const QUrl &url );

    /** @returns the currently specified timeout in milliseconds. */
    int timeout() const;

    /**
     * Sets timeout in milliseconds. Once the specified time has elapsed, the current
     * query is aborted.
     *
     * @param millis Query timeout in milliseconds
     */
    void setTimeout( int millis );

signals:
    /**
     * Emitted when a search has been completed.
     * @param success true if the search was completed successfully.
     */
    void finished( bool success );

public slots:
    /**
     * Search the wiki for the specified search term.
     */
    void search( const QString &searchTerm );

    /**
     * Aborts the currently running request.
     */
    void abort();

private slots:
    void finished( QNetworkReply *reply );

private:
    void findBase();
    bool processBaseResult( QIODevice *source );
    bool processSearchResult( QIODevice *source );

    struct MediaWikiPrivate * const d;
};

#endif // MEDIAWIKI_H
