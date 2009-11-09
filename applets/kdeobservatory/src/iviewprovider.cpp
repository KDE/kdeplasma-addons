#include "iviewprovider.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsRectItem>

#include <Plasma/Frame>

IViewProvider::IViewProvider(const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: QObject(),
  m_rect(rect),
  m_parent(parent),
  m_wFlags(wFlags)
{
}

IViewProvider::~IViewProvider()
{
}

QGraphicsWidget *IViewProvider::createView(const QString &title)
{
    QGraphicsWidget *view = new QGraphicsWidget(m_parent);
    view->hide();

    Plasma::Frame *header = new Plasma::Frame(view);
    header->setText(title);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGraphicsWidget *container = new QGraphicsWidget(view);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, view);
    layout->addItem(header);
    layout->addItem(container);

    view->setLayout(layout);
    view->setGeometry(m_rect);

    m_views.append(view);
    return container;
}

const QList<QGraphicsWidget *> IViewProvider::views() const
{
    return m_views;
}
