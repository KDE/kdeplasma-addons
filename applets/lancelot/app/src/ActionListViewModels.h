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

#ifndef ACTIONLISTVIEWMODELS_H_
#define ACTIONLISTVIEWMODELS_H_

#include <QPair>
#include <QString>
#include <KIcon>
#include <QVariant>

namespace Lancelot
{

class ActionListViewModel: public QObject {
    Q_OBJECT
public:
    ActionListViewModel();
    virtual ~ActionListViewModel();

    virtual QString title(int index) const = 0;
    virtual QString description(int index) const;
    virtual KIcon * icon(int index) const;
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

class StandardActionListViewModel: public ActionListViewModel {
    Q_OBJECT
protected:
    class Item {
    public:
        Item(QString itemTitle, QString itemDescription, KIcon * itemIcon, QVariant itemData)
          : title(itemTitle), description(itemDescription), icon(itemIcon), data(itemData) {};

        QString title;
        QString description;
        KIcon * icon;
        QVariant data;
    };

public:
    StandardActionListViewModel();
    virtual ~StandardActionListViewModel();

    virtual QString title(int index) const;
    virtual QString description(int index) const;
    virtual KIcon * icon(int index) const;
    virtual bool isCategory(int index) const;

    virtual int size() const;

    void add(const Item & item);
    void add(const QString & title, const QString & description, KIcon * icon, const QVariant & data);

    void set(int index, const Item & item);
    void set(int index, const QString & title, const QString & description, KIcon * icon, const QVariant & data);
    void removeAt(int index);

protected:

    QList < Item > m_items;

};

class MergedActionListViewModel: public ActionListViewModel {
    Q_OBJECT
public:
    MergedActionListViewModel();
    virtual ~MergedActionListViewModel();

    void addModel(KIcon * icon, QString title, ActionListViewModel * model);

    int modelCount() const;

    // ActionListViewModel methods

    virtual QString title(int index) const;
    virtual QString description(int index) const;
    virtual KIcon * icon(int index) const;
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
    QList< QPair< QString, KIcon * > > m_modelsMetadata;
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

class DummyActionListViewModel : public ActionListViewModel {
public:
    DummyActionListViewModel(QString title, int size)
        : Lancelot::ActionListViewModel(), m_size(size), m_title(title), m_icon(new KIcon("lancelot")) {}

    virtual ~DummyActionListViewModel() {}

    virtual QString title(int index) const {
        return m_title + " " + QString::number(index) + " long long long long long " + QString((index < size())?"":"err");
    }

    virtual QString description(int index) const {
        return "Description " + QString::number(index);
    }
    virtual KIcon * icon(int index) const { Q_UNUSED(index); return m_icon; }
    virtual int size() const { return m_size; }

    virtual void activated(int index) { Q_UNUSED(index); }
private:
    int m_size;
    QString m_title;
    KIcon * m_icon;
};

class DummyMergedActionListViewModel : public MergedActionListViewModel {
public:
    DummyMergedActionListViewModel (QString title, int subs) {
        while (subs-- != 0) {
            addModel(NULL, title + QString::number(subs), new DummyActionListViewModel(title + QString::number(subs), 2 + subs % 2));
        }
    }
};


}

#endif /*ACTIONLISTVIEWMODELS_H_*/
