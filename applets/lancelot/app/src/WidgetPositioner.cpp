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

void WidgetPositioner::setGeometry(const QRectF & geometry)
{
    m_geometry = geometry;
    if (m_widget) {
        QPoint pos = m_view->mapFromScene(mapToScene(m_geometry.topLeft()));
        m_widget->setGeometry(QRect(pos, geometry.size().toSize()));
        //m_widget->setGeometry(sceneBoundingRect().toRect());
    }
}

QRectF WidgetPositioner::geometry() const
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
