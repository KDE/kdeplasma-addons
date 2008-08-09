/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOTAPP_MODELS_BASEMODEL_H_
#define LANCELOTAPP_MODELS_BASEMODEL_H_

#include <lancelot/models/ActionListViewModels.h>
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

class BaseModel : public Lancelot::StandardActionListViewModel {
    Q_OBJECT
public:
    BaseModel();
    virtual ~BaseModel();

protected:
    virtual void activate(int index);
    virtual void load() = 0;

    bool addService(const QString & service);
    bool addService(const KService::Ptr & service);

    void addServices(const QStringList & services);

    bool addUrl(const QString & url);
    bool addUrl(const KUrl & url);

    void addUrls(const QStringList & urls);

    void hideLancelotWindow();
    void changeLancelotSearchString(const QString & string);
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_BASEMODEL_H_ */
