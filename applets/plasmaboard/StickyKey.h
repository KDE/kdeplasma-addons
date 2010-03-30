#ifndef STICKYKEY_H
#define STICKYKEY_H

#include "FuncKey.h"

class StickyKey : public FuncKey
{
public:
    StickyKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString label);

    virtual void pressed();
    virtual void released();
    virtual void reset();
    virtual void setPixmap(QPixmap *pixmap);
    virtual void unpress();

private:
    bool m_acceptPixmap;
    bool m_dorelease;
    bool m_toggled;


};

#endif // STICKYKEY_H
