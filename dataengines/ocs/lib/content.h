/*
    This file is part of KDE.

    Copyright (c) 2008 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef ATTICA_CONTENT_H
#define ATTICA_CONTENT_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSharedDataPointer>

#include "atticaclient_export.h"


class QDateTime;

namespace Attica {

/**
 * Represents a single content
 */
class ATTICA_EXPORT Content
{
  public:
    typedef QList<Content> List;
    class Parser;
  
    /**
     * Creates an empty Content
     */
    Content();

    /**
     * Copy constructor.
     * @param other the Content to copy from
     */
    Content(const Content& other);

    /**
     * Assignment operator.
     * @param other the Content to assign from
     * @return pointer to this Content
     */
    Content& operator=(const Content& other);

    /**
     * Destructor.
     */
    ~Content();

    /**
     * Sets the id of the Content.
     * The id uniquely identifies a Content with the OCS API.
     * @param id the new id
     */
    void setId(const QString& id);

    /**
     * Gets the id of the Content.
     * The id uniquely identifies a Content with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the name of the Content.
     * @param name the new name
     */
    void setName(const QString& name);

    /**
     * Gets the name of the Content.
     * @return the name
     */
    QString name() const;

    /**
     * Sets the rating of the Content.
     * @param rating the new rating, has to be in the range 0-100
     */
    void setRating(int rating);

    /**
     * Gets the rating of the Content.
     * @return the rating in the range 0-100
     */
    int rating() const;
    
    /**
     * Sets the number of downloads for the Content.
     * @param downloads the new number of downloads
     */
    void setDownloads(int downloads);
    
    /**
     * Gets the number of downloads for the Content.
     * @return the number of downloads
     */
    int downloads() const;
    
    /**
     * Sets the date and time the Content has been created.
     * @param created the new creation date and time
     */
    void setCreated(const QDateTime& created);
    
    /**
     * Gets the date and time the Content has been created.
     * @return the date and time of the last update
     */
    QDateTime created() const;

    /**
     * Sets the time the Content has been last updated.
     * @param updated the new date and time of the last update
     */
    void setUpdated(const QDateTime& updated);

    /**
     * Gets the date and time the Content has been last updated.
     * @return the date and time of the last update
     */
    QDateTime updated() const;

    /**
     * Add an attribute that is not included in the basis set of attributes exposed by the Content class.
     * If the attribute already exists it gets overwritten.
     * @param key the key of the attribute
     * @param value the value of the attribute
     */
    void addExtendedAttribute( const QString &key, const QString &value );

    /**
     * Get an attribute that is not included in the basis set of attributes exposed by the Content class.
     * @param key the key of the attribute
     * @return the value of the attribute with the specified key, or an empty string, if the key has not been found
     */
    QString extendedAttribute( const QString &key ) const;

    /**
     * Get all attributes that are not included in the basis set of attributes exposed by the Content class.
     * @return the attribute mappings
     */
    QMap<QString,QString> extendedAttributes() const;

    /**
     * Checks whether this Content has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
