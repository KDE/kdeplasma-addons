/*
 *   SPDX-FileCopyrightText: 2009 Richard Moore <rich@kde.org>
 *   SPDX-FileCopyrightText: 2009 Sebastian Kügler <sebas@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef MEDIAWIKI_H
#define MEDIAWIKI_H

// Qt
#include <QObject>
#include <QList>
#include <QUrl>

class QNetworkReply;
class QIODevice;

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
        }

        Result( const Result &r ) {
            this->title = r.title;
            this->url = r.url;
        }

        Result &operator= (const Result &other)  {
            this->title = other.title;
            this->url = other.url;

            return *this;
        }

        /** The page title of the match. */
        QString title;
        /** The URL of the page containing the match. */
        QUrl url;
    };

    /**
     * Create a media wiki querying object with the specified parent. The querying
     * object can be used for multiple queries, though only one can be performed at
     * a time.
     * @param parent The parent object
     */
    explicit MediaWiki(QObject *parent = nullptr);
    ~MediaWiki() override;

    /**
     * @returns a list of matches.
     */
    QList<MediaWiki::Result> results() const;

    /** @returns the currently specified maximum number of results to return. */
    int maxItems() const;

    /** Sets the maximum number of results to return.
     *
     * @param limit Maximum number number of results to retrieve
     */
    void setMaxItems( int limit );

    /** @returns the currently specified API URL. */
    QUrl apiUrl() const;

    /**
     * Sets the URL at which the wikis API page can be found. For example, wikipedia
     * has the API file at https://en.wikipedia.org/w/api.php whilst techbase has the
     *
     * @param url The URL of the api.php file, for example https://techbase.kde.org/api.php
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

Q_SIGNALS:
    /**
     * Emitted when a search has been completed.
     * @param success true if the search was completed successfully.
     */
    void finished( bool success );

public Q_SLOTS:
    /**
     * Search the wiki for the specified search term.
     */
    void search( const QString &searchTerm );

    /**
     * Aborts the currently running request.
     */
    void abort();

private Q_SLOTS:
    void onNetworkRequestFinished(QNetworkReply *reply);

private:
    void findBase();
    bool processBaseResult( QIODevice *source );
    bool processSearchResult( QIODevice *source );

    struct MediaWikiPrivate * const d;
};

#endif // MEDIAWIKI_H
