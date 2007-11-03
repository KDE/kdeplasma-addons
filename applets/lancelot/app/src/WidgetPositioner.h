#ifndef WIDGETPOSITIONER_H_
#define WIDGETPOSITIONER_H_

#include "Widget.h"
#include <QWidget>
#include <QGraphicsView>

namespace Lancelot
{

/**
 * Dirty hack to show QWidget as a part of QGV. It works for the
 * case it is used. It is not meant to be used elsewhere.
 */
class WidgetPositioner: public Widget
{
public:
	WidgetPositioner(QWidget * widget, QGraphicsView * view, QGraphicsItem * parent);
	virtual ~WidgetPositioner();

    void setGeometry (const QRectF & geometry);
	QRectF geometry();

    QSizeF sizeHint() const;

    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);


private:
    QRectF m_geometry;
    QWidget * m_widget;
    QGraphicsView * m_view;
};

}

#endif /*WIDGETPOSITIONER_H_*/
