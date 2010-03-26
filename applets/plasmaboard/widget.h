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

#include <plasma/containment.h>
#include <plasma/dataengine.h>

#include <QXmlStreamReader>

#define XK_TECHNICAL
#define XK_PUBLISHING
#define XK_LATIN1
#include <X11/keysym.h>

class AlphaNumKey;
class BoardKey;
class FuncKey;

namespace {
    class DataEngine;
    class FrameSvg;
}

class PlasmaboardWidget : public Plasma::Containment
{
    Q_OBJECT
public:

    PlasmaboardWidget(QGraphicsWidget *parent);
    ~PlasmaboardWidget();

    /**
      * Clears lock key and calls clear()
      */
    void clearAnything();

    /**
      * Draws just basic keys on the keyboard - just for writing
      */
    void initKeyboard(const QString &file);

    /**
      * The paintInterface procedure paints the applet to screen
      */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

    /**
      * Deletes all keys for resetting the keyboard
      */
    void resetKeyboard();

protected:
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    virtual void resizeEvent ( QGraphicsSceneResizeEvent * event );
    //virtual bool event ( QEvent * event );

private:
    /**
      * Removes tooltip
      */
    void clearTooltip();
    FuncKey *createFunctionKey(QPoint &point, QSize &size, QString action, QString label);
    QPixmap *getActiveFrame(const QSize &size);
    QPixmap *getFrame(const QSize &size);
    void press(BoardKey* key);
    void release(BoardKey* key);
    void unpress(BoardKey* key);

public Q_SLOTS:

    /**
      * Unsets all pressed keys despite of caps
      */
    void clear();

    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

    /**
      * Triggers a relabeling of alphanumeric keys on the keyboard
      */
    void relabelKeys();
    void refreshKeys();

    /**
      * Sets tooltip to a new text
      */
    void setTooltip(BoardKey* key);
    void themeChanged();

signals:
    void shiftKey(bool value);
    void altKey(bool value);
    void altGrKey(bool value);
    void superKey(bool value);
    void controlKey(bool value);
    void menuKey(bool value);


private:
    Plasma::FrameSvg* m_activeFrame;
    QHash<QSize, QPixmap*> m_activeFrames;
    QList<AlphaNumKey*> m_alphaKeys; // normal keys labeled with symbols like a, b, c
    Plasma::DataEngine* m_engine;
    Plasma::FrameSvg* m_frame;
    QHash<QSize, QPixmap*> m_frames;
    QList<FuncKey*> m_funcKeys; // functional keys like shift, backspace, enter
    bool m_isAlternative; // alternative key level activated
    bool m_isLevel2; // second key level activated
    bool m_isLocked; // is lock activated
    QList<BoardKey*> m_keys;
    QList<BoardKey*> m_pressedList;
    Tooltip* m_tooltip;
    QXmlStreamReader m_xmlReader;
};

inline uint qHash(const QSize &key)
{
    return qHash(key.width()) + qHash(key.height());
}

#endif /* WIDGET_H */
