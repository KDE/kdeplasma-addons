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
#ifndef ATTICA_CATEGORY_H
#define ATTICA_CATEGORY_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QList>

#include "atticaclient_export.h"


namespace Attica {


/**
 * Represents a single content category
 */
class ATTICA_EXPORT Category
{
  public:
    typedef QList<Category> List;
    class Parser;
  
    /**
     * Creates an empty Category
     */
    Category();

    /**
     * Copy constructor.
     * @param other the Category to copy from
     */
    Category(const Category& other);

    /**
     * Assignment operator.
     * @param other the Category to assign from
     * @return pointer to this Category
     */
    Category& operator=(const Category& other);

    /**
     * Destructor.
     */
    ~Category();


    /**
     * Sets the id of the Category.
     * The id uniquely identifies a Category with the OCS API.
     * @param id the new id
     */
    void setId( const QString & );

    /**
     * Gets the id of the Category.
     * The id uniquely identifies a Category with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the name of the Category.
     * @param name the new name
     */
    void setName(const QString& name);

    /**
     * Gets the name of the Category.
     * @return the name
     */
    QString name() const;

    /**
     * Checks whether this Category has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
