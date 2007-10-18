#ifndef ACTIONLISTVIEWMODELS_H_
#define ACTIONLISTVIEWMODELS_H_

#include <QPair>
#include <QString>
#include <KIcon>
#include <QMessageBox>

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

    virtual void activated(int index);
    
    virtual int size() const = 0;
    
Q_SIGNALS:
    void itemActivated(int index);

    void updated();
    void itemInserted(int index);
    void itemDeleted(int index);
    void itemAltered(int index);
};

class ActionListViewMergedModel: public ActionListViewModel {
    Q_OBJECT
public:
    ActionListViewMergedModel();
    virtual ~ActionListViewMergedModel();
    
    void addModel(KIcon * icon, QString title, ActionListViewModel * model);

    int modelCount() const;

    // ActionListViewModel methods

    virtual QString title(int index) const;
    virtual QString description(int index) const;
    virtual KIcon * icon(int index) const;
    virtual bool isCategory(int index) const;
    virtual int size() const;

    virtual void activated(int index);
    
private:
    void toChildCoordinates(int index, int & model, int & modelIndex) const;
    void fromChildCoordinates(int & index, int model, int modelIndex) const;
    QList< ActionListViewModel * > m_models;
    QList< QPair< QString, KIcon * > > m_modelsMetadata;

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
        return m_title + QString::number(index) + QString((index < size())?"":"err"); 
    }
    
    virtual QString description(int index) const {
        return "Description " + QString::number(index); 
    }
    virtual KIcon * icon(int index) const { Q_UNUSED(index); return m_icon; }
    virtual int size() const { return m_size; }
    
    virtual void activated(int index) {
        QMessageBox::information (NULL, "Item is activated", title(index));
    }
private:
    int m_size;
    QString m_title;
    KIcon * m_icon;
};

class DummyActionListViewMergedModel : public ActionListViewMergedModel {
public:
    DummyActionListViewMergedModel (QString title, int subs) {
        while (subs-- != 0) {
            addModel(NULL, title + QString::number(subs), new DummyActionListViewModel(title + QString::number(subs), subs + 2));
        }
    }
};


}

#endif /*ACTIONLISTVIEWMODELS_H_*/
