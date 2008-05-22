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

#ifndef LANCELOT_ACTION_LIST_VIEW_MODELS_H_
#define LANCELOT_ACTION_LIST_VIEW_MODELS_H_

#include <lancelot/lancelot_export.h>

#include <QPair>
#include <QString>
#include <QIcon>
#include <QVariant>

namespace Lancelot
{

class LANCELOT_EXPORT ActionListViewModel: public QObject {
    Q_OBJECT
public:
    ActionListViewModel();
    virtual ~ActionListViewModel();

    virtual QString title(int index) const = 0;
    virtual QString description(int index) const;
    virtual QIcon icon(int index) const;
    virtual bool isCategory(int index) const;

    virtual int size() const = 0;

public slots:
    void activated(int index);

Q_SIGNALS:
    void itemActivated(int index);

    void updated();
    void itemInserted(int index);
    void itemDeleted(int index);
    void itemAltered(int index);

protected:
    /** Models should reimplement this function. It is invoked when
     *  an item is activated, before the itemActivated signal is emitted */
    virtual void activate(int index) { Q_UNUSED(index); };
};

class LANCELOT_EXPORT StandardActionListViewModel: public ActionListViewModel {
    Q_OBJECT
protected:
    class LANCELOT_EXPORT Item {
    public:
        Item(QString itemTitle, QString itemDescription, QIcon itemIcon, QVariant itemData)
          : title(itemTitle), description(itemDescription), icon(itemIcon), data(itemData) {};

        QString title;
        QString description;
        QIcon icon;
        QVariant data;
    };

public:
    StandardActionListViewModel();
    virtual ~StandardActionListViewModel();

    virtual QString title(int index) const;
    virtual QString description(int index) const;
    virtual QIcon icon(int index) const;
    virtual bool isCategory(int index) const;

    virtual int size() const;

    void add(const Item & item);
    void add(const QString & title, const QString & description, QIcon icon, const QVariant & data);

    void set(int index, const Item & item);
    void set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data);
    void removeAt(int index);

protected:

    QList < Item > m_items;

};

class LANCELOT_EXPORT MergedActionListViewModel: public ActionListViewModel {
    Q_OBJECT
public:
    MergedActionListViewModel();
    virtual ~MergedActionListViewModel();

    void addModel(QIcon icon, const QString & title, ActionListViewModel * model);

    int modelCount() const;

    // ActionListViewModel methods

    virtual QString title(int index) const;
    virtual QString description(int index) const;
    virtual QIcon icon(int index) const;
    virtual bool isCategory(int index) const;
    virtual int size() const;

    bool hideEmptyModels() const;
    void setHideEmptyModels(bool hide);

protected:
    virtual void activate(int index);

private:
    void toChildCoordinates(int index, int & model, int & modelIndex) const;
    void fromChildCoordinates(int & index, int model, int modelIndex) const;
    QList< ActionListViewModel * > m_models;
    QList< QPair< QString, QIcon > > m_modelsMetadata;
    bool m_hideEmptyModels;

private slots:
    // listen to model changes
    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);


Q_SIGNALS:
    void itemActivated(int index);

    void updated();
    void itemInserted(int index);
    void itemDeleted(int index);
    void itemAltered(int index);
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_MODELS_H_ */

