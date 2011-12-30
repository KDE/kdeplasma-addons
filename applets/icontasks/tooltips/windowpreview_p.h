/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
 *   Copyright 2011 Craig Drummond <craig@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_WINDOWPREVIEW_P_H
#define PLASMA_WINDOWPREVIEW_P_H

#include <QWidget> // base class
#include <QSize> // stack allocated

#include <plasma/framesvg.h>
#include "tooltipcontent.h"

using namespace Plasma;

namespace IconTasks
{

/**
 * @internal
 *
 * A widget which reserves area for window preview and sets hints on the toplevel
 * tooltip widget that tells KWin to render the preview in this area. This depends
 * on KWin's TaskbarThumbnail compositing effect (which is automatically detected).
 */
class WindowPreview : public QWidget
{
    Q_OBJECT

public:
//     static bool previewsAvailable();
    static int setPreviewSize(int size);

    WindowPreview(QWidget *parent = 0);

    void setWindows(const QList<ToolTipContent::Window> &wins);
    void setInfo();
    bool isEmpty() const;
    virtual QSize sizeHint() const;
    void setHighlightWindows(const bool highlight);
    void setVertical(const bool v);
    bool highlightWindows() const;

Q_SIGNALS:
    void windowPreviewClicked(WId wid, Qt::MouseButtons buttons, Qt::KeyboardModifiers keys, const QPoint &screenPos);
    void windowButtonClicked(WId wid, Qt::MouseButtons buttons, Qt::KeyboardModifiers keys, const QPoint &screenPos);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void controlButtons(QMouseEvent *event);

private:

    struct Rects {
        QRect thumb;
        QRect icon;
        QRect button;
        QRect text;
        QRect sub;
        QRect hover;
    };

    QList<ToolTipContent::Window> m_windows;
    QList<WId> m_windowIds;
    mutable QList<QSize> m_windowSizes;
    QList <Rects> m_rects;
    mutable QList<QSize> m_itemSizes;
    mutable QMap<int, int> m_rowSizes;
    mutable QMap<int, int> m_columnSizes;
    FrameSvg *m_background;
    bool m_highlightWindows;
    QPixmap m_closePixmap;
    QPixmap m_hoverClosePixmap;
    QPixmap m_disabledClosePixmap;
    QPoint m_pos;
    Qt::MouseButtons m_btns;
    int m_textHeight;
    int m_subTextHeight;
    int m_hoverThumbnailId;
    int m_hoverBtnId;
    int m_rows;
    int m_columns;
    int m_maxRows;
    int m_maxColumns;
    bool m_vertical;
    bool m_showDesktop;
    int m_moreWindows;

    static const int WINDOW_MARGIN = 10;
};

} // namespace Plasma

#endif // PLASMA_WINDOWPREVIEW_P_H

