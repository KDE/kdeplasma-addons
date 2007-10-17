#ifndef LANCELOT_WIDGET_H_
#define LANCELOT_WIDGET_H_

#include <plasma/widgets/widget.h>

namespace Lancelot
{

class Widget : public Plasma::Widget
{
public:
	Widget(QString name, QGraphicsItem * parent = 0);
	virtual ~Widget();
	
    void setGeometry (const QRectF & geometry);
    void update (const QRectF &rect = QRectF());
    void update (qreal x, qreal y, qreal w, qreal h);
    
    QString name() const;
    void setName(QString name);

protected:
    QString m_name;
};

}

#endif /*LANCELOT_WIDGET_H_*/
