#include "iview.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsRectItem>

#include <Plasma/Frame>

IView::IView(const QString &title, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: QGraphicsWidget(parent, wFlags)
{
    m_header = new Plasma::Frame(this);
    m_header->setText(title);
    m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_container = new QGraphicsWidget(this);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->addItem(m_header);
    layout->addItem(m_container);
    setLayout(layout);
    setGeometry(rect);
}

IView::~IView()
{
}
