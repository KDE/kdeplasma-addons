#include "ToggleExtenderButton.h"
#include <KDebug>

namespace Lancelot
{

ToggleExtenderButton::ToggleExtenderButton(QString name, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, title, description, parent), m_pressed(false)
{
    init();
}

ToggleExtenderButton::ToggleExtenderButton(QString name, QIcon * icon, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent), m_pressed(false)
{
    init();
}

ToggleExtenderButton::ToggleExtenderButton(QString name, Plasma::Svg * icon, QString title, QString description, QGraphicsItem * parent)
  : ExtenderButton(name, icon, title, description, parent), m_pressed(false)
{
    init();
}

void ToggleExtenderButton::init()
{
    kDebug() << "Init\n";
    connect (this, SIGNAL(activated()), this, SLOT(toggle()));
}

bool ToggleExtenderButton::isPressed() const
{
    return m_pressed;
}

void ToggleExtenderButton::setPressed(bool pressed)
{
    if (pressed == m_pressed) return;
    
    emit toggled(m_pressed = pressed);
    update();
}

void ToggleExtenderButton::toggle()
{
    emit toggled(m_pressed = !m_pressed);
    update();
}

ToggleExtenderButton::~ToggleExtenderButton()
{
}

void ToggleExtenderButton::paintWidget (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    paintBackground(painter);
    if (m_pressed) {
        paintBackground(painter, "pressed");
    }
    paintForeground(painter);
}


}

#include "ToggleExtenderButton.moc"
