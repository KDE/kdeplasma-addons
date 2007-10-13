#ifndef PANEL_H_
#define PANEL_H_

#include <plasma/widgets/widget.h>
#include <plasma/widgets/layout.h>
#include "BaseWidget.h"

#include <QIcon>

namespace Lancelot
{

class Panel: public Plasma::Widget
{
public:
    Panel(QString name, QIcon * icon, QString title = QString(), QGraphicsItem * parent = 0);
    Panel(QString name, QString title, QGraphicsItem * parent = 0);
    Panel(QString name, QGraphicsItem * parent = 0);
    
	virtual ~Panel();
	
	void setTitle(QString & title);
	QString title() const;
	
	void setIcon(QIcon * icon);
	QIcon * icon() const;

	void setIconSize(QSize size);
    QSize iconSize() const;
	
    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);   

    //void resize (const QSizeF &size);
    //void resize (qreal width, qreal height);
    void setGeometry (const QRectF & geometry);
    
    void setLayout(Plasma::LayoutItem * layout);
    Plasma::LayoutItem * layout();
    
    void setWidget(Plasma::Widget * widget);
    Plasma::Widget * widget();
    
    void setVisible(bool visible);
    void show();
    void hide();

private:
    void init();
    void invalidate();
    
    Plasma::LayoutItem * m_layout;
    Plasma::Widget * m_widget;
    bool m_hasTitle;
    QString m_name;
    
    BaseWidget m_titleWidget;
};

}

#endif /*PANEL_H_*/
