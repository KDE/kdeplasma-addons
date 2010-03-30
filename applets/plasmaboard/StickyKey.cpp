#include "StickyKey.h"

StickyKey::StickyKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString label):
        FuncKey(relativePosition, relativeSize, keycode, label)
{
    m_toggled = false;
    m_dorelease = false;
    m_acceptPixmap = true;
}

void StickyKey::pressed()
{
    m_acceptPixmap = true;
    if(!m_toggled){
        sendKeyPress(); // if the key has not pressed, send immediately a press to X server
        m_toggled = true;
    }
    else {
        m_toggled = false;
        m_dorelease = true;
    }
}

void StickyKey::released()
{
    if(m_dorelease){
        sendKeyRelease();
        m_dorelease = false;
        m_acceptPixmap = true;
    }
    else {
        m_acceptPixmap = false;
    }
}

void StickyKey::reset()
{
    m_acceptPixmap = true;
    if(m_toggled){
        sendKeyRelease();
        m_toggled = false;
    }
}

void StickyKey::setPixmap(QPixmap *pixmap)
{
    if(!m_acceptPixmap){
        return; // if toggled we want to keep the pressed pixmap
    }
    FuncKey::setPixmap(pixmap);
}

void StickyKey::unpress()
{
    m_acceptPixmap = true;
    if(m_toggled && !m_dorelease){
        m_toggled = false;
        sendKeyRelease();
    }
    else if(!m_toggled && m_dorelease){
        m_toggled = true;
        m_dorelease = false;
    }
}
