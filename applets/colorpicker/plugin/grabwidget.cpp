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
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

#include <KWindowSystem>

Q_DECLARE_METATYPE(QColor)

QDBusArgument &operator<< (QDBusArgument &argument, const QColor &color)
{
    argument.beginStructure();
    argument << color.rgba();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QColor &color)
{
    argument.beginStructure();
    QRgb rgba;
    argument >> rgba;
    argument.endStructure();
    color = QColor::fromRgba(rgba);
    return argument;
}

Grabber::Grabber(QObject *parent)
    : QObject(parent)
{
}

Grabber::~Grabber() = default;

void Grabber::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }
    m_color = color;
    emit colorChanged();
}

X11Grabber::X11Grabber(QObject *parent)
    : Grabber(parent)
    , m_grabWidget(new QWidget(nullptr, Qt::BypassWindowManagerHint))
{
    m_grabWidget->move(-5000, -5000);
}

X11Grabber::~X11Grabber()
{
    delete m_grabWidget;
}

void X11Grabber::pick()
{
    // TODO pretend the mouse went somewhere else to prevent the tooltip from spawning

    m_grabWidget->show();
    m_grabWidget->installEventFilter(this);
    m_grabWidget->grabMouse(Qt::CrossCursor);
}

bool X11Grabber::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease) {
        m_grabWidget->removeEventFilter(this);
        m_grabWidget->hide();
        m_grabWidget->releaseMouse();

        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        if (me->button() == Qt::LeftButton) {
            const QPoint pos = me->globalPos();
            const QDesktopWidget *desktop = QApplication::desktop();
            const QPixmap pixmap = QGuiApplication::screens().at(desktop->screenNumber())->grabWindow(desktop->winId(),
                                                                                                      pos.x(), pos.y(), 1, 1);
            if (!pixmap.isNull()) {
                QImage i = pixmap.toImage();
                QColor color(i.pixel(0, 0));
                setColor(color);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

KWinWaylandGrabber::KWinWaylandGrabber(QObject *parent)
    : Grabber(parent)
{
    qDBusRegisterMetaType<QColor>();
}

KWinWaylandGrabber::~KWinWaylandGrabber() = default;

void KWinWaylandGrabber::pick()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/ColorPicker"),
                                                      QStringLiteral("org.kde.kwin.ColorPicker"),
                                                      QStringLiteral("pick"));
    auto call = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
        [this] (QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            QDBusPendingReply<QColor> reply = *watcher;
            if (!reply.isError()) {
                setColor(reply.value());
            }
        }
    );
}

GrabWidget::GrabWidget(QObject *parent)
    : QObject(parent)
{
    if (KWindowSystem::isPlatformX11()) {
        m_grabber = new X11Grabber(this);
    } else if (KWindowSystem::isPlatformWayland()) {
        m_grabber = new KWinWaylandGrabber(this);
    }
    if (m_grabber) {
        connect(m_grabber, &Grabber::colorChanged, this, &GrabWidget::currentColorChanged);
    }
}

GrabWidget::~GrabWidget() = default;

QColor GrabWidget::currentColor() const
{
    return m_grabber ? m_grabber->color() : QColor();
}

void GrabWidget::pick()
{
    if (m_grabber) {
        m_grabber->pick();
    }
}

void GrabWidget::copyToClipboard(const QString &text)
{
    QApplication::clipboard()->setText(text);
}
