#include "WidgetPositioner.h"

namespace Lancelot
{

WidgetPositioner::WidgetPositioner(QWidget * widget, QGraphicsView * view, QGraphicsItem * parent)
    : Widget("WidgetPositioner", parent), m_widget(widget), m_view(view)
{
}

WidgetPositioner::~WidgetPositioner()
{
}

void WidgetPositioner::paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(QColor(0,0,0)));
}

void WidgetPositioner::setGeometry(const QRectF & geometry)
{
    m_geometry = geometry;
    if (m_widget) {
        QPoint pos = m_view->mapFromScene(mapToScene(m_geometry.topLeft()));
        m_widget->setGeometry(QRect(pos, geometry.size().toSize()));
        //m_widget->setGeometry(sceneBoundingRect().toRect());
    }
}

QRectF WidgetPositioner::geometry()
{
    return m_geometry;
}

QSizeF WidgetPositioner::sizeHint() const
{
    if (m_widget) {
        return m_widget->sizeHint();
    }
    return QSizeF();
}


}
