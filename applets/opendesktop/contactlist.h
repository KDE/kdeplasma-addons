#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QtCore/QString>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/DataEngine>
#include <Plasma/ScrollWidget>

#include "contactwidget.h"

/**
 * The ContactList class provides a Plasma widget for displaying lists of contactwidget
 */
class ContactList : public Plasma::ScrollWidget
{
    Q_OBJECT
    
    public:
        /**
         * Creates a new ContactList widget without entries
         * @param engine the Plasma data engine to use
         * @param parent the parent of this widget
         */
        ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

        /**
         * Returns the maximum number of items shown
         * @return the current limit
         */
        int limit() const;

        /**
         * Returns the query whose results should be displayed
         * @return the current query
         */
        QString query() const;

        /**
         * Adjusts the maximum number of items shown
         * @param limit the new limit
         */
        void setLimit(int limit);

        /**
         * Sets the query whose results should be displayed
         * @param query a source of the data engine that can be interpreted as a list of contacts (where each person is identified by a Person-[id] key)
         */
        void setQuery(const QString& query);

    Q_SIGNALS:
        /**
         * This signal is emitted when the user requests the details of a contact
         * @param id the identification of the requested contact details
         */
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
