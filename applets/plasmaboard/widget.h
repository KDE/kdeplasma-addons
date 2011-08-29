/****************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef WIDGET_H
#define WIDGET_H

#include "tooltip.h"

#include <QGraphicsWidget>
#include <QXmlStreamReader>
#include <plasma/popupapplet.h>
#include <plasma/dataengine.h>

#define XK_TECHNICAL
#define XK_PUBLISHING
#define XK_LATIN1
#include <X11/keysym.h>

class AlphaNumKey;
class BoardKey;
class DualKey;
class FuncKey;
class QSignalMapper;
class QTimer;
class StickyKey;
class SwitchKey;

namespace
{
class DataEngine;
class FrameSvg;
}

/**
 * @class PlasmaboardWidget applets/plasmaboard/widget.h
 *
 * @short The main widget of the onscreen keyboard
 *
 * Widget provides main functionality of the onscreen keyboard
 *
 */

class PlasmaboardWidget : public QGraphicsWidget
{
    Q_OBJECT
public:
    enum StateAction { NoActions = 0, Press = 1, Unpress = 2, Reset = 4, Release = 8, ExternalEvent = 16};
    Q_DECLARE_FLAGS(StateActions, StateAction)

    PlasmaboardWidget(Plasma::PopupApplet *parent);
    ~PlasmaboardWidget();

    /**
      * Deletes all keys for resetting the keyboard
      */
    void deleteKeys();

    /**
      * Draws just basic keys on the keyboard - just for writing
      * @param Absolute path to the file which contains xml layout of keyboard
      */
    void initKeyboard(const QString &file);

    /**
      * The paintInterface procedure paints the applet to screen
      */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

    /**
      * Resets the keyboard. Unpresses all pressed keys and releases all sticking functional keys
      */
    void reset();

    /**
      *
      */
    void switchAlternative(bool alt);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void resizeEvent(QGraphicsSceneResizeEvent * event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

private:
    /**
      * Presses or unpresses function keys
      * @param FuncKey to act on
      * @param state to change to. True: press, False: unpress
      */
    template<typename T>
    void setKeysState(const QList<T> &keys, const StateActions &actions);

    /**
      * Hides Tooltip
      */
    void clearTooltip();

    /**
      * @param Position of the key
      * @param Size of the key
      * @param String which classifys the functional key that should be returned
      * @return FuncKey corresponding to the action
      */
    FuncKey *createFunctionKey(const QPoint &point, const QSize &size, const QString &action);

    /**
      * @param Position of the key
      * @param Size of the key
      * @param String which classifys the functional key that should be returned
      * @return FuncKey corresponding to the action
      */
    FuncKey *createStickyKey(const QPoint &point, const QSize &size, const QString &action);

    enum BackgroundState { ActiveBackground, NormalBackground };

    /**
      * @param state The state of the background to find and return
      * @param size Size of the pixmap requested
      * @return QPixmap in pressed state with correct size
      */
    QPixmap *background(BackgroundState state, const QSize &size);

    /**
      * Presses given key. Calls key->press() sets the needed pixmap and show tooltip
      * @param key to act on
      */
    void press(BoardKey* key, bool externalEvent = false);

    /**
      * Releases given key. In most cases this actually sends the token to the X-server
      * @param Key to act on
      */
    void release(BoardKey* key);

    /**
      * Unpresses given key. This undoes the press without actually sending anything to the X-Server
      * @param Key to act on
      */
    void unpress(BoardKey* key);

public Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

    /**
      * Triggers a relabeling of alphanumeric keys on the keyboard respecting key modifiers
      */
    void relabelKeys();

    /**
      * Presses all pressed keys so that they can be repeated
      */
    void repeatKeys();

    /**
      * Is called after a AlphaNumeric key is released. It unpresses all pressed modifier keys
      * Currently it unpresses all other pressed keys too. FIXME for multi touch support
      */
    void refreshKeys();

    /**
      * Sets tooltip to a new text, new size, new position and show it
      * @param key to show the tooltip for
      */
    void setTooltip(BoardKey* key);

    /**
      * Called when the plasma theme changes
      * Clears pixmap caches and triggers redraw of keyboard
      */
    void themeChanged();

protected Q_SLOTS:
    void stickyKey_Mapper(int id);
    void showToolTip();

private:
    QHash<QSize, QPixmap*> m_activeFrames; // cache of all pixmap sizes with active state
    QList<AlphaNumKey*> m_alphaKeys; // normal keys labeled with symbols like a, b, c
    Plasma::PopupApplet *m_applet;
    QSize m_keyboardSize; // size configured in keyboard XML file
    QList<StickyKey*> m_altKeys; // List of all AltKeys on keyboard
    QList<StickyKey*> m_altgrKeys; // List of all AltGr keys on keyboard
    QList<FuncKey*> m_capsKeys; // List of all caps keys on keyboard
    QList<StickyKey*> m_ctlKeys; // List of Control keys on keyboard
    QList<DualKey*> m_dualKeys; // List of dual keys on keyboard
    Plasma::DataEngine* m_engine; // key state data engine
    Plasma::FrameSvg* m_frameSvg; // svg in normal state
    QHash<QSize, QPixmap*> m_frames; // cace of all pixmap sizes with normal state
    QList<FuncKey*> m_funcKeys; // functional keys like shift, backspace, enter
    bool m_isAlternative; // alternative key level activated
    bool m_isLevel2; // second key level activated
    bool m_isLocked; // is lock activated
    bool m_isRepeating;
    QList<BoardKey*> m_keys; // list of all keys displayed
    QList<BoardKey*> m_pressedList; // list all currently pressed keys
    QTimer* m_repeatTimer;
    QTimer* m_delayedToolTipShow;
    QSignalMapper* m_signalMapper;
    QList<StickyKey*> m_shiftKeys; // list of Shift-Keys on keyboard
    QList<FuncKey*> m_specialKeys; // list of special keys like Backspace, F[1-12], Enter and so on
    QMap<int, BoardKey*> m_stickyKeys; // list of keys waiting for being unpressed
    QList<StickyKey*> m_superKeys; // list of all super-keys on keyboard
    QList<SwitchKey*> m_switchKeys; // list of all switch keys on keyboard
    Tooltip* m_tooltip; // pointer to widget which is used as tooltip
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlasmaboardWidget::StateActions)

inline uint qHash(const QSize &key)
{
    return qHash(key.width()) + qHash(key.height());
}

#endif /* WIDGET_H */
