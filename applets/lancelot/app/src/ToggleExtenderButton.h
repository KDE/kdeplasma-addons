#ifndef TOGGLEEXTENDERBUTTON_H_
#define TOGGLEEXTENDERBUTTON_H_

#include "ExtenderButton.h"

namespace Lancelot
{

class ToggleExtenderButton : public Lancelot::ExtenderButton {
    Q_OBJECT
public:
    ToggleExtenderButton(QString name = QString(), QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ToggleExtenderButton(QString name, QIcon * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ToggleExtenderButton(QString name, Plasma::Svg * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);

    virtual ~ToggleExtenderButton();
    
    bool isPressed() const;
    void setPressed(bool pressed = true);
    
    virtual void paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    
public slots:
    void toggle();

Q_SIGNALS:
    void toggled(bool pressed);
    
private:
    bool m_pressed;
    void init();
    
};

}

#endif /*TOGGLEEXTENDERBUTTON_H_*/
