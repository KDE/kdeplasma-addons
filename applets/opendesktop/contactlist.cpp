#include "contactlist.h"


ContactList::ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent),
      m_engine(engine),
      m_limit(20)
{
    m_container = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_container);
    setWidget(m_container);
}


QString ContactList::query() const
{
    return m_query;
}


void ContactList::setQuery(const QString& query)
{
    if (query != m_query) {
        if (!m_query.isEmpty()) {
            m_engine->disconnectSource(m_query, this);
        }
        m_query = query;
        if (!m_query.isEmpty()) {
            m_engine->connectSource(m_query, this);
            dataUpdated(m_query, m_engine->query(m_query));
        }
    }
}


QStringList ContactList::getDisplayedContacts(const Plasma::DataEngine::Data& data) {
    // FIXME: This should take into order filtering, sorting etc.
    QStringList result;
    foreach (const QString& key, data.keys()) {
        if (result.size() >= m_limit) {
            break;
        }
        if (key.startsWith("Person-")) {
            result.append(QString(key).remove(0, 7));
        }
    }
    return result;
}


void ContactList::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source)
    
    QStringList displayedContacts = getDisplayedContacts(data);
    
    // FIXME: This is still highly inefficient
    while (m_layout->count()) {
        ContactWidget* widget = static_cast<ContactWidget*>(m_layout->itemAt(0));
        m_layout->removeAt(0);
        widget->deleteLater();
    }
    m_mapping.clear();

    QStringList::iterator j = displayedContacts.begin();
    for (int i = 0; i < displayedContacts.size(); ++i, ++j) {
        ContactWidget* widget;
        widget = new ContactWidget(m_container);
        widget->setAtticaData(data[QString("Person-%1").arg(*j)].value<Plasma::DataEngine::Data>());
        m_layout->addItem(widget);
        m_mapping.insert(widget, *j);
        connect(widget, SIGNAL(showDetails(Plasma::DataEngine::Data)), SLOT(showDetails()));
    }
    this->setPos(0, 0);
}


int ContactList::limit() const
{
    return m_limit;
}


void ContactList::setLimit(int limit)
{
    if (limit != m_limit) {
        m_limit = limit;
        dataUpdated(m_query, m_engine->query(m_query));
    }
}


void ContactList::showDetails()
{
    ContactWidget* widget = static_cast<ContactWidget*>(sender());
    emit showDetails(m_mapping[widget]);
}


#include "contactlist.moc"
