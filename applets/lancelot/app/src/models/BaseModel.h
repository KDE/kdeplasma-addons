/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTAPP_MODELS_BASEMODEL_H
#define LANCELOTAPP_MODELS_BASEMODEL_H

#include <lancelot/models/StandardActionListModel.h>
#include <KService>
#include <KUrl>

namespace Models {

class ApplicationConnector: public QObject {
    Q_OBJECT
public:
    static ApplicationConnector * instance();
    void search(const QString & search);
    bool hide(bool immediate = false);

Q_SIGNALS:
    void doSearch(const QString & search);
    bool doHide(bool immediate);

private:
    ApplicationConnector();
    ~ApplicationConnector();

    static ApplicationConnector * m_instance;
};

class BaseModel : public Lancelot::StandardActionListModel {
    Q_OBJECT
public:
    BaseModel(bool enableDefaultDnD = false);
    virtual ~BaseModel();

    L_Override virtual QMimeData * mimeData(int index) const;
    L_Override virtual void setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);

protected:
    virtual void activate(int index);
    virtual void load() = 0;

    bool addService(const QString & service);
    bool addService(const KService::Ptr & service);

    int addServices(const QStringList & services);

    bool addUrl(const QString & url);
    bool addUrl(const KUrl & url);

    int addUrls(const QStringList & urls);

    void hideLancelotWindow();
    void changeLancelotSearchString(const QString & string);

public:
    static QMimeData * mimeForUrl(const KUrl & url);
    static QMimeData * mimeForUrl(const QString & url);

    static QMimeData * mimeForService(const KService::Ptr & service);
    static QMimeData * mimeForService(const QString & service);

private:
    bool m_enableDefaultDnD;
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_BASEMODEL_H */
