#ifndef LANCELOT_PANEL_H_
#define LANCELOT_PANEL_H_

#include "Widget.h"
#include <plasma/widgets/layout.h>
#include "BaseActionWidget.h"

#include <QIcon>

namespace Lancelot
{

class Panel: public Widget
{
public:
    Panel(QString name, QIcon * icon, QString title = QString(), QGraphicsItem * parent = 0);
    Panel(QString name, QString title, QGraphicsItem * parent = 0);
    Panel(QString name, QGraphicsItem * parent = 0);

	virtual ~Panel();

	void setTitle(const QString & title);
	QString title() const;

	void setIcon(QIcon * icon);
	QIcon * icon() const;

	void setIconSize(QSize size);
    QSize iconSize() const;

    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    void setGeometry (const QRectF & geometry);

    void setLayout(Plasma::LayoutItem * layout);
    Plasma::LayoutItem * layout();

    void setWidget(Widget * widget);
    Widget * widget();

    void setVisible(bool visible);
    void show();
    void hide();

private:
    void init();
    void invalidate();

    Plasma::LayoutItem * m_layout;
    Widget * m_widget;
    bool m_hasTitle;

    BaseActionWidget m_titleWidget;
};

}

#endif /*PANEL_H_*/
