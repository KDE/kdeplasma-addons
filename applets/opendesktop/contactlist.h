#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QtCore/QString>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/DataEngine>
#include <Plasma/ScrollWidget>

#include "contactwidget.h"


class ContactList : public Plasma::ScrollWidget
{
    Q_OBJECT
    
    public:
        ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);
        int limit() const;
        QString query() const;
        void setLimit(int limit);
        void setQuery(const QString& query);

    Q_SIGNALS:
        void showDetails(const QString& id);
        
    private Q_SLOTS:
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);
        void showDetails();

    private:
        QStringList getDisplayedContacts(const Plasma::DataEngine::Data& data);
        
        QGraphicsWidget* m_container;
        Plasma::DataEngine* m_engine;
        QGraphicsLinearLayout* m_layout;
        int m_limit;
        QHash<ContactWidget*, QString> m_mapping;
        QString m_query;
};


#endif
