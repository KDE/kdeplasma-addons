#include "Widget.h"
#include "Global.h"

namespace Lancelot
{

Widget::Widget(QString name, QGraphicsItem * parent)
  : Plasma::Widget(parent), m_name(name)
{
}

Widget::~Widget()
{
}

QString Widget::name() const {
    return m_name;
}

void Widget::setName(QString name) {
    m_name = name;
}

void Widget::setGeometry (const QRectF & geometry) {
    if (!Global::processGeometryChanges) return;
    Plasma::Widget::setGeometry(geometry);
}

void Widget::update (const QRectF &rect) {
    if (!Global::processUpdateRequests) return;
    Plasma::Widget::update(rect);
}

void Widget::update (qreal x, qreal y, qreal w, qreal h) {
    if (!Global::processUpdateRequests) return;
    Plasma::Widget::update(x, y, w, h);
}

}
