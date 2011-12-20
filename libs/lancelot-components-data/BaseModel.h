/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_DATA_BASEMODEL_H
#define LANCELOT_DATA_BASEMODEL_H

#include <KService>

#include <QAbstractListModel>
#include <QModelIndex>

#include <KUrl>
#include <QIcon>

/**
 * Implements a few useful methods for Lancelot::StandardActionListModel
 */
class BaseModel: public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QString selfTitle READ selfTitle CONSTANT)
    Q_PROPERTY(QIcon   selfIcon  READ selfIcon  CONSTANT)
    Q_PROPERTY(int     count     READ count     NOTIFY countChanged)

protected:
    class Item {
    public:
        explicit Item(QString itemTitle = QString(),
                QString itemDescription = QString(),
                QIcon itemIcon = QIcon(),
                QVariant itemData = QVariant()
            )
            : title(itemTitle), description(itemDescription), icon(itemIcon), data(itemData) {};

        QString title;
        QString description;
        QIcon icon;
        QVariant data;
    };

public:
    /**
     * Creates a new BaseModel
     */
    BaseModel(const QString & title, const QIcon & icon);

    /**
     * Destroys this BaseModel
     */
    virtual ~BaseModel();

    /**
     * @returns the title of the model
     */
    QString selfTitle() const;

    /**
     * @returns the icon of the model
     */
    QIcon selfIcon() const;

    /**
     * @returns the number of the items in the model
     */
    int count() const;

    /**
     * @returns the number of the items in the model
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * @returns the data
     */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;


Q_SIGNALS:
    /**
     * Emitted when the number of items in the model is changed
     */
    void countChanged();


protected:
    /**
     * Reimplement this function to load the model data
     */
    virtual void load() = 0;

    /**
     * Adds a service or application to the model
     * @param service name of the service
     */
    bool addService(const QString & service);

    /**
     * Adds a service or application to the model
     * @param service service to add
     */
    bool addService(const KService::Ptr & service);

    /**
     * Adds a list of services or applications to the model
     * @param service names of the services
     */
    int addServices(const QStringList & services);

    /**
     * Adds an url to the model
     * @param url url to add
     */
    bool addUrl(const QString & url);

    /**
     * Adds an url to the model
     * @param url url to add
     */
    bool addUrl(const KUrl & url);

    /**
     * Adds a list of urls to the model
     * @param url url to add
     */
    int addUrls(const QStringList & urls);

    /**
     * Adds a new item into the list
     * @param item item to add
     */
    void add(const Item & item);

    /**
     * Adds a new item into the list
     * @param title title for the item
     * @param description description of the item
     * @param icon icon for the item
     * @param data data for the item
     */
    void add(const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Inserts a service or application to the model
     * @param where where to insert the service
     * @param service name of the service
     */
    bool insertService(int where, const QString & service);

    /**
     * Inserts a service or application to the model
     * @param where where to insert the service
     * @param service service
     */
    bool insertService(int where, const KService::Ptr & service);

    /**
     * Inserts a list of services or applications to the model
     * @param where where to insert the services
     * @param services names of the services
     */
    int insertServices(int where, const QStringList & services);

    /**
     * Inserts an url to the model
     * @param where where to insert the url
     * @param url url to add
     */
    bool insertUrl(int where, const QString & url);

    /**
     * Inserts an url to the model
     * @param where where to insert the url
     * @param url url to add
     */
    bool insertUrl(int where, const KUrl & url);

    /**
     * Inserts a list of urls to the model
     * @param where where to insert the urls
     * @param urls urls to add
     */
    int insertUrls(int where, const QStringList & urls);

    /**
     * Inserts a new item at a specified location
     * @param where location where to add the new item
     * @param item item to add
     */
    void insert(int where, const Item & item);

    /**
     * Inserts a new item at a specified location
     * @param where location where to add the new item
     * @param title title for the item
     * @param description description of the item
     * @param icon icon for the item
     * @param data data for the item
     *
     */
    void insert(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Replaces the specified item with a new one
     * @param where location of the item to replace
     * @param item new value for the item
     */
    void set(int where, const Item & item);

    /**
     * Replaces the specified item with a new one
     * @param where location of the item to replace
     * @param title title for the item
     * @param description description of the item
     * @param icon icon for the item
     * @param data data for the item
     */
    void set(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Removes an item at a specified location
     * @param index index of the item to remove
     */
    void removeAt(int index);

    /**
     * @returns item at a specified location
     * @param index item index
     */
    const Item & itemAt(int index);

    /**
     * Removes all items from the model
     */
    void clear();

    /**
     * Sets the title for the model
     */
    void setSelfTitle(const QString & title);

    /**
     * Sets the icon for the model
     */
    void setSelfIcon(const QIcon & icon);


private:
    class Private;
    Private * const d;
};


#endif /* LANCELOT_DATA_BASEMODEL_H */
