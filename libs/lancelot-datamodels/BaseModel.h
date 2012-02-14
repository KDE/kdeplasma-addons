/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef LANCELOTAPP_MODELS_BASEMODEL_H
#define LANCELOTAPP_MODELS_BASEMODEL_H

#include <lancelot/lancelot_export.h>

#include <lancelot/models/StandardActionListModel.h>
#include <KService>
#include <KUrl>

namespace Lancelot {
namespace Models {

/**
 * Common interface from models to the parent application
 */
class LANCELOT_EXPORT ApplicationConnector: public QObject {
    Q_OBJECT
public:
    /**
     * @returns singleton instance of ApplicationConnector
     */
    static ApplicationConnector * self();

    /**
     * Requests a search
     * @param search string to search for
     */
    void search(const QString & search);

    /**
     * Requests window hide
     * @param immediate whether there shouldn't be any
     *     delays before hiding
     */
    void hide(bool immediate = false);

    /**
     * Sets whether hide() should be ignored
     * @param value if true - hide() will not be ignored
     */
    void setAutohideEnabled(bool value);

    /**
     * @see setAutohideEnabled()
     */
    bool autohideEnabled() const;

    /**
     * @returns whether the application name should be in the first line,
     * and the description in the second
     */
    bool applicationNameFirst() const;

Q_SIGNALS:
    /**
     * This signal is emitted when searching is requested
     * @param search string to search for
     */
    void doSearch(const QString & search);

    /**
     * This signal is emitted when hiding is requested
     * @param immediate whether there shouldn't be any
     *     delays before hiding
     */
    void doHide(bool immediate);

private:
    ApplicationConnector();
    ~ApplicationConnector();

    static ApplicationConnector * m_instance;

    class Private;
    Private * const d;
};

/**
 * Implements a few useful methods for Lancelot::StandardActionListModel
 */
class LANCELOT_EXPORT BaseModel : public Lancelot::StandardActionListModel {
    Q_OBJECT
public:
    /**
     * Creates a new BaseModel
     * @param enableDefaultDnD whether default drag and drop
     *      handling should be turned on
     */
    BaseModel(bool enableDefaultDnD = false);

    /**
     * Destroys this BaseModel
     */
    virtual ~BaseModel();

    L_Override QMimeData * mimeData(int index) const;
    L_Override void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);

    L_Override QString selfTitle() const;
    L_Override QIcon selfIcon() const;
    L_Override QMimeData * selfMimeData() const;

protected:
    virtual void activate(int index);

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
     * Hides the parent application window
     * (if the parent application listens to
     * ApplicationConnector signals)
     */
    static void hideApplicationWindow();

    /**
     * Changed the search string of the parent
     * application (if the parent application listens to
     * ApplicationConnector signals)
     */
    static void changeApplicationSearchString(const QString & string);

    /**
     * Sets the title for the model
     */
    void setSelfTitle(const QString & title);

    /**
     * Sets the icon for the model
     */
    void setSelfIcon(const QIcon & icon);

    /**
     * Sets the mime data for the model
     */
    void setSelfMimeData(QMimeData * data);

public:
    /**
     * @param url url
     * @returns the mime data for the specified url
     */
    static QMimeData * mimeForUrl(const KUrl & url);

    /**
     * @param url url
     * @returns the mime data for the specified url
     */
    static QMimeData * mimeForUrl(const QString & url);

    /**
     * @param service service
     * @returns the mime data for the specified service
     */
    static QMimeData * mimeForService(const KService::Ptr & service);

    /**
     * @param service service
     * @returns the mime data for the specified service
     */
    static QMimeData * mimeForService(const QString & service);

private:
    class Private;
    Private * const d;
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_BASEMODEL_H */
