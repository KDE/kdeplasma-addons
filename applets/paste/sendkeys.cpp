/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sendkeys.h"
#include <KGlobal>
#include <KDebug>
#include <QMap>
#include <QKeySequence>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/keysym.h>

class SendKeysPrivate {
public:
    SendKeysPrivate()
    {
        keys[Qt::Key_F1] = XK_F1;
        keys[Qt::Key_F2] = XK_F2;
        keys[Qt::Key_F3] = XK_F3;
        keys[Qt::Key_F4] = XK_F4;
        keys[Qt::Key_F5] = XK_F5;
        keys[Qt::Key_F6] = XK_F6;
        keys[Qt::Key_F7] = XK_F7;
        keys[Qt::Key_F8] = XK_F8;
        keys[Qt::Key_F9] = XK_F9;
        keys[Qt::Key_F10] = XK_F10;
        keys[Qt::Key_F11] = XK_F11;
        keys[Qt::Key_F12] = XK_F12;
        // Add more if needed
    };

    QMap<int, int> keys;
};

SendKeys::SendKeys()
    : d(new SendKeysPrivate())
{
}

SendKeys::~SendKeys()
{
    delete d;
}

SendKeys &SendKeys::self()
{
     K_GLOBAL_STATIC(SendKeys, s_instance)
     return *s_instance;
}

SendKeys &SendKeys::operator<<(const QString &string)
{
    send(string);
    return *this;
}

SendKeys &SendKeys::operator<<(uint k)
{
    send(k);
    return *this;
}

SendKeys &SendKeys::operator<<(const QKeySequence &ks)
{
   send(ks);
   return *this;
}

void SendKeys::send(const QString &string)
{
    foreach(uint key, string.toUcs4()) {
        send(key);
    }
}

void SendKeys::send(const QKeySequence &ks)
{
   for (uint i = 0; i < ks.count(); ++i) {
        send(ks[i]);
    }
}

void SendKeys::send(uint k)
{
    Display *dsp = QX11Info::display();
    uint keycode = k & 0x01FFFFFF;
    XKeyEvent event;
    Window currentFocus;
    int focusState;

    if (d->keys.contains(keycode)) {
        keycode = d->keys[keycode];
    } else if (keycode < Qt::Key_Space && keycode > Qt::Key_ydiaeresis) {
        return;
    }
    keycode = XKeysymToKeycode(dsp, keycode);

    event.display = dsp;
    XGetInputFocus(dsp, &currentFocus, &focusState);
    event.window = currentFocus;
    event.root = DefaultRootWindow(dsp);
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 0;
    event.y = 0;
    event.x_root = 0;
    event.y_root = 0;
    event.same_screen = true;
    event.type = KeyPress;
    event.keycode = keycode;
    event.state = 0;
    if (k & Qt::ALT) {
        event.state |= Mod1Mask;
    }
    if (k & Qt::CTRL) {
        event.state |= ControlMask;
    }
    if (k & Qt::META) {
        event.state |= Mod1Mask;
    }
    if (k & Qt::SHIFT) {
        event.state |= ShiftMask;
    }
    XSendEvent(dsp, InputFocus, false, KeyPressMask, (XEvent *)&event);

    event.type = KeyRelease;
    event.time = CurrentTime;
    XSendEvent(dsp, InputFocus, false, KeyReleaseMask, (XEvent *)&event);
}
