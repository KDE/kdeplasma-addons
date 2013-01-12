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

#include "windowpreview_p.h"
#include "tooltipcontent.h"
#include "tooltipmanager.h"
#include <QPainter>
#include <QVarLengthArray>
#include <QMouseEvent>
#include <QFontMetrics>

#include <kwindowsystem.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

#include <plasma/framesvg.h>
#include <plasma/svg.h>
#include <plasma/windoweffects.h>
#include <plasma/paintutils.h>

#ifdef Q_WS_X11
#include <QX11Info>
#include <QDesktopWidget>

#include <X11/Xlib.h>
#include <fixx11h.h>
#endif
#include <math.h>

namespace IconTasks
{

WindowPreview::WindowPreview(QWidget *parent)
    : QWidget(parent),
      m_highlightWindows(false)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/tasks");
    m_background->setElementPrefix("normal");
    setMouseTracking(true);
    const QString svgPath("widgets/configuration-icons");
    if (Plasma::Theme::defaultTheme()->imagePath(svgPath).isEmpty()) {
        m_closePixmap = KIcon("window-close").pixmap(ToolTipContent::iconSize(), ToolTipContent::iconSize());
    } else {
        Plasma::Svg svg(this);
        svg.setImagePath(svgPath);
        m_closePixmap = svg.pixmap(QLatin1String("close")).scaled(ToolTipContent::iconSize(), ToolTipContent::iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    KIconEffect *effect = KIconLoader::global()->iconEffect();
    if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState)) {
        m_hoverClosePixmap = effect->apply(m_closePixmap, KIconLoader::Desktop, KIconLoader::ActiveState);
    }
    if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::DisabledState)) {
        m_disabledClosePixmap = effect->apply(m_closePixmap, KIconLoader::Desktop, KIconLoader::DisabledState);
    }

    m_textHeight = QFontMetrics(font()).height() + 6;
    m_subTextHeight = QFontMetrics(KGlobalSettings::smallestReadableFont()).height() + 4;
    m_hoverThumbnailId = m_hoverBtnId = -1;

    m_maxRows = m_rows = 1;
    m_maxColumns = m_columns = 4;
}

void WindowPreview::setWindows(const QList<ToolTipContent::Window> &wins)
{
//     if (!WindowEffects::isEffectAvailable(WindowEffects::WindowPreview)) {
//         setMinimumSize(0,0);
//         setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
//         m_windowIds.clear();
//         m_windows.clear();
//         return;
//     }

    bool havePreviews = WindowEffects::isEffectAvailable(WindowEffects::WindowPreview);

#ifdef Q_WS_X11
    static const int constMargin = 48;
    QRect screenSize(QApplication::desktop()->screen(x11Info().screen())->geometry().adjusted(constMargin, constMargin, -constMargin, -constMargin));
    int titleSpace = qMax(ToolTipContent::iconSize(), m_textHeight);

    m_maxColumns = m_columns = qMax(4, screenSize.width() / (ToolTipManager::self()->previewWidth() + WINDOW_MARGIN));
    m_maxRows = m_rows = qMax(1, screenSize.height() / ((havePreviews ? ToolTipManager::self()->previewHeight() : 0) + WINDOW_MARGIN + titleSpace));
#endif

    int max = m_maxRows * m_maxColumns;

    if (wins.count() <= max) {
        m_moreWindows = 0;
        m_windows = wins;

        // Work out actual rows/columns...
        bool vertical = m_vertical || !havePreviews;
        int orientMax = vertical ? m_maxRows : m_maxColumns;
        if (wins.count() > orientMax) {
            double optimum = sqrt((double)wins.count());
            m_rows = m_columns = optimum > ((int)optimum) ? ((int)optimum + 1) : ((int)optimum);

            if (m_rows > m_maxRows) {
                m_rows = m_maxRows;
                m_columns = (wins.count() / (double)m_rows) + 0.5;
            }
            if (m_columns > m_maxColumns) {
                m_columns = m_maxColumns;
                m_rows = (wins.count() / (double)m_columns) + 0.5;
            }
        } else if (vertical) {
            m_rows = wins.count();
            m_columns = 1;
        } else {
            m_columns = wins.count();
            m_rows = 1;
        }
    } else {
        m_moreWindows = wins.count() - max;
        m_windows = wins.mid(0, max);
    }

    m_windowIds.clear();
    m_showDesktop = false;
    foreach (ToolTipContent::Window w, m_windows) {
        m_windowIds.append(w.id);
        if (0 != w.desktop) {
            m_showDesktop = true;
        }
    }

    if (havePreviews) {
        m_windowSizes = WindowEffects::windowSizes(m_windowIds);
    } else {
        m_windowSizes.clear();
    }
    QSize s(sizeHint());
    if (s.isValid()) {
        setFixedSize(sizeHint());
    }
}

QSize WindowPreview::sizeHint() const
{
    if (m_windowIds.size() == 0) {
        return QSize();
    }

    m_rowSizes.clear();
    m_columnSizes.clear();
    m_itemSizes.clear();
    int column = 0;
    int row = 0;
    bool havePreviews = WindowEffects::isEffectAvailable(WindowEffects::WindowPreview);

    if (havePreviews) {
        if (!m_windowSizes.size() == 0) {
            m_windowSizes = WindowEffects::windowSizes(m_windowIds);
        }

        int maxHeight = 0;

        if ((1 == m_columns && m_rows > 1) || (1 == m_rows && m_columns > 1)) {
            foreach (const QSize & s, m_windowSizes) {
                maxHeight = qMax(s.height(), maxHeight);
            }
        }

        foreach (const QSize & s, m_windowSizes) {
            QSize sz(s);
            sz.scale(ToolTipManager::self()->previewWidth(), maxHeight ? maxHeight : ToolTipManager::self()->previewHeight(), Qt::KeepAspectRatio);
            if (sz.height() > ToolTipManager::self()->previewHeight()) {
                sz.scale(ToolTipManager::self()->previewWidth(), ToolTipManager::self()->previewHeight(), Qt::KeepAspectRatio);
            }
            m_itemSizes.append(sz);
            if (m_rowSizes[row] < sz.height()) {
                m_rowSizes[row] = sz.height();
            }
            if (m_columnSizes[column] < sz.width()) {
                m_columnSizes[column] = sz.width();
            }
            if (++column == m_columns) {
                column = 0;
                row++;
            }
        }
    } else {
        for (int i = 0; i < m_windowIds.size(); ++i) {
            m_rowSizes[row] = 0;
            m_columnSizes[column] = ToolTipManager::DEF_PREVIEW_SIZE;
            m_itemSizes.append(QSize(m_columnSizes[column], m_rowSizes[row]));;
            if (++column == m_columns) {
                column = 0;
                row++;
            }
        }
    }

    QSize sz(0, 0);
    int titleSpace = qMax(ToolTipContent::iconSize(), m_textHeight);

    foreach (int v, m_rowSizes.values()) {
        sz = sz + QSize(0, v + (WINDOW_MARGIN) + titleSpace + (havePreviews ? (WINDOW_MARGIN / 2.0) : 0.0));
        if (m_showDesktop) {
            sz = sz + QSize(0, m_subTextHeight);
        }
    }

    foreach (int v, m_columnSizes.values()) {
        sz = sz + QSize(v + WINDOW_MARGIN, 0);
    }

    if (m_moreWindows) {
        sz = sz + QSize(0, WINDOW_MARGIN + m_textHeight);
    }
    return sz;
}


bool WindowPreview::isEmpty() const
{
    foreach (WId id, m_windowIds) {
        if (id != 0) {
            return false;
        }
    }

    return true;
}


void WindowPreview::setHighlightWindows(const bool highlight)
{
    m_highlightWindows = highlight;
}

void WindowPreview::setVertical(const bool v)
{
    m_vertical = v;
}

bool WindowPreview::highlightWindows() const
{
    return m_highlightWindows;
}

void WindowPreview::setInfo()
{
    QWidget *w = parentWidget();
    if (isEmpty()) {
        WindowEffects::showWindowThumbnails(w->winId());
        return;
    }

    bool havePreviews = WindowEffects::isEffectAvailable(WindowEffects::WindowPreview);

    if (havePreviews) {
        if (m_windowSizes.size() == 0) {
            m_windowSizes = WindowEffects::windowSizes(m_windowIds);
        }

        if (m_windowSizes.size() == 0) {
            WindowEffects::showWindowThumbnails(w->winId());
            return;
        }
    }

    Q_ASSERT(w->isWindow()); // parent must be toplevel

    QSize thumbnailSize = sizeHint();
    thumbnailSize.scale(size(), Qt::KeepAspectRatio);
    m_background->resizeFrame(thumbnailSize);

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    left -= 2; top -= 2; right -= 2; bottom -= 2;
    const QRect thumbnailRect(QPoint(left, top), size() - QSize(left + right, top + bottom));
    QList<QRect> inParentCoords;
    m_rects.clear();

    int x = thumbnailRect.x();
    int y = thumbnailRect.y();
    int column = 0;
    int row = 0;
    int titleHeight = qMax(ToolTipContent::iconSize(), m_textHeight);
    int titleSpace = titleHeight + (WINDOW_MARGIN / 2.0);
    bool rtl = QApplication::layoutDirection() == Qt::RightToLeft;

    foreach (QSize s, m_itemSizes) {
        int width = m_columnSizes[column];
        int height = m_rowSizes[row];
        Rects rects;

        rects.thumb = QRect(QPoint(x + (1 == m_columns && m_rows > 1 ? 0 : (width - s.width()) / 2), y + titleSpace), s);
        rects.button = QRect((x + width) - ToolTipContent::iconSize(), y + (titleHeight - ToolTipContent::iconSize()) / 2, ToolTipContent::iconSize(), ToolTipContent::iconSize());
        rects.icon = QRect(x, y + (titleHeight - ToolTipContent::iconSize()) / 2, ToolTipContent::iconSize(), ToolTipContent::iconSize());
        if (rtl) {
            QRect a = rects.button;
            rects.button = rects.icon;
            rects.icon = a;
        }
        rects.text = QRect(x + ToolTipContent::iconSize() + 2, y, width - ((ToolTipContent::iconSize() + 2) * 2), titleHeight);
        if (m_showDesktop) {
            rects.sub = QRect(rtl ? rects.thumb.left() : rects.text.left(),
                              rects.text.bottom(),
                              rects.thumb.width() - (ToolTipContent::iconSize() + 2),
                              m_subTextHeight);
            rects.thumb.adjust(0, m_subTextHeight, 0, m_subTextHeight);
        }
        inParentCoords.append(QRect(mapToParent(rects.thumb.topLeft()), s));
        rects.hover = rects.thumb.adjusted(-left, -top, right, bottom).united(rects.icon).united(rects.button);
        m_rects.append(rects);
        x += width + WINDOW_MARGIN;
        if (++column == m_columns) {
            column = 0;
            row++;
            x = thumbnailRect.x();
            y += height + (havePreviews ? WINDOW_MARGIN : (WINDOW_MARGIN / 2.0)) + titleSpace + (m_showDesktop ? m_subTextHeight : 0);
        }
    }

    if (havePreviews) {
        WindowEffects::showWindowThumbnails(w->winId(), m_windowIds, inParentCoords);
    }
    m_hoverThumbnailId = m_hoverBtnId = -1;
    repaint();
}

void WindowPreview::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    QWidget *w = parentWidget();

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    left -= 2; top -= 2; right -= 2; bottom -= 2;
    const QSize delta(left + right, top + bottom);
    const QPoint topLeft(left, top);
    bool havePreviews = WindowEffects::isEffectAvailable(WindowEffects::WindowPreview);
    int i = 0;

    QFont f(font());
    QFontMetrics fm(f);
    QFont small = KGlobalSettings::smallestReadableFont();
    small.setItalic(true);
    QFontMetrics smallFm(small);
    painter.setFont(font());

    foreach (Rects rects, m_rects) {
        if (havePreviews) {
            m_background->setElementPrefix(i == m_hoverBtnId || i == m_hoverThumbnailId ? "hover" : "normal");
            m_background->resizeFrame(rects.thumb.size() + delta);
            m_background->paintFrame(&painter, rects.thumb.topLeft() - topLeft);
        }

        if (i == m_hoverBtnId || i == m_hoverThumbnailId || !m_disabledClosePixmap.isNull()) {
            painter.drawPixmap(rects.button.x(), rects.button.y(), i == m_hoverBtnId
                               ? m_hoverClosePixmap
                               : i == m_hoverThumbnailId
                               ? m_closePixmap
                               : m_disabledClosePixmap);
        }
        QString s(fm.elidedText(m_windows[i].text, Qt::ElideRight, rects.text.width(), QPalette::WindowText));
        QString sub;

        if (m_showDesktop && 0 != m_windows[i].desktop) {
            sub = -1 == m_windows[i].desktop
                  ? i18n("(On All Desktops)")
                  : i18nc("Which virtual desktop a window is currently on", "(On %1)", KWindowSystem::desktopName(m_windows[i].desktop));
        }

        painter.drawText(rects.text, s, QTextOption(Qt::AlignVCenter));
        if (m_windows[i].attention) {
            painter.drawText(rects.text.adjusted(1, 0, 1, 0), s, QTextOption(Qt::AlignVCenter));
        }
        if (!sub.isEmpty()) {
            painter.setFont(small);
            painter.drawText(rects.sub.adjusted(1, 0, 1, 0), sub, QTextOption(Qt::AlignVCenter));
            painter.setFont(f);
        }
        painter.drawPixmap(rects.icon.x(), rects.icon.y(), m_windows[i].image);
        i++;
    }

    if (m_moreWindows) {
        QRect windowRect = w->rect();
        int height = fm.height();
        QRect textRect = QRect(windowRect.x() + left, windowRect.bottom() - ((WINDOW_MARGIN * 2.5) + height + bottom), windowRect.width() - ((2 * (WINDOW_MARGIN)) + left + right), height);
        QString s(i18n("Plus %1 more...", m_moreWindows));
        f.setItalic(true);
        painter.setFont(f);
        painter.drawText(textRect, s, QTextOption(Qt::AlignVCenter));
    }
}

void WindowPreview::mousePressEvent(QMouseEvent *event)
{
    m_pos = event->pos();
    m_btns = event->buttons();
}

void WindowPreview::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pos.isNull()) {
        return;
    }

    for (int i = 0; i < m_rects.size(); ++i) {
        if (m_rects[i].button.contains(event->pos()) && m_rects[i].button.contains(m_pos)) {
            emit windowButtonClicked(m_windowIds[i], m_btns, event->modifiers(), event->globalPos());
            break;
        } else if ((m_rects[i].hover.contains(event->pos()) && m_rects[i].hover.contains(m_pos))) {
            emit windowPreviewClicked(m_windowIds[i], m_btns, event->modifiers(), event->globalPos());
            break;
        }

    }
    m_pos = QPoint();
}

void WindowPreview::mouseMoveEvent(QMouseEvent *event)
{
    controlButtons(event);
    if (!m_highlightWindows) {
        return;
    }

    for (int i = 0; i < m_rects.size(); ++i) {
        if (m_rects[i].hover.contains(event->pos())) {
            WindowEffects::highlightWindows(effectiveWinId(), QList<WId>() << effectiveWinId() << m_windowIds[i]);
            return;
        }
    }

    WindowEffects::highlightWindows(effectiveWinId(), QList<WId>());
}

void WindowPreview::leaveEvent(QEvent *event)
{
    controlButtons(0L);
    Q_UNUSED(event)
    if (!m_highlightWindows) {
        return;
    }
    WindowEffects::highlightWindows(effectiveWinId(), QList<WId>());
}

void WindowPreview::controlButtons(QMouseEvent *event)
{
    bool needUpdate = !event && (m_hoverThumbnailId >= 0 || m_hoverThumbnailId >= 0);

    if (!needUpdate && event) {
        int hoverThumbnailId = -1, hoverBtnId = -1;

        for (int i = 0; i < m_rects.size(); ++i) {
            if (m_rects[i].hover.contains(event->pos())) {
                hoverThumbnailId = i;
                break;
            }
        }

        if (!m_hoverClosePixmap.isNull()) {
            for (int i = 0; i < m_rects.size(); ++i) {
                if (m_rects[i].button.contains(event->pos())) {
                    hoverBtnId = i;
                    break;
                }
            }
        }

        needUpdate = ((m_hoverThumbnailId >= 0 || hoverThumbnailId >= 0) && m_hoverThumbnailId != hoverThumbnailId) ||
                     ((m_hoverBtnId >= 0 || hoverBtnId >= 0) && m_hoverBtnId != hoverBtnId);
        m_hoverThumbnailId = hoverThumbnailId;
        m_hoverBtnId = hoverBtnId;
    }

    if (needUpdate) {
        if (!event) {
            m_hoverThumbnailId = m_hoverBtnId = -1;
        }
        repaint();
    }
}

} // namespace Plasma

#include "windowpreview_p.moc"
