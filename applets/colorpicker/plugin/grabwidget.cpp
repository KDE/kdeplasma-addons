/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "grabwidget.h"

#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

GrabWidget::GrabWidget(QObject *parent)
    : QObject(parent)
    , m_grabWidget(new QWidget(nullptr, Qt::BypassWindowManagerHint))
{
    m_grabWidget->move(-5000, -5000);
}

GrabWidget::~GrabWidget()
{
    delete m_grabWidget;
}

QColor GrabWidget::currentColor() const
{
    return m_currentColor;
}

void GrabWidget::pick()
{
    // TODO pretend the mouse went somewhere else to prevent the tooltip from spawning

    m_grabWidget->show();
    m_grabWidget->installEventFilter(this);
    m_grabWidget->grabMouse(Qt::CrossCursor);
}

void GrabWidget::copyToClipboard(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

bool GrabWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease) {
        m_grabWidget->removeEventFilter(this);
        m_grabWidget->hide();
        m_grabWidget->releaseMouse();

        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        const QPoint pos = me->globalPos();

        foreach (QScreen *screen, QApplication::screens()) {
            if (screen->geometry().contains(pos)) {
                const QPixmap pixmap = screen->grabWindow(0);
                const QPoint localPos = (pos - screen->geometry().topLeft()) * qApp->devicePixelRatio();
                m_currentColor = QColor(pixmap.toImage().pixel(localPos));
                emit currentColorChanged();
                break;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
