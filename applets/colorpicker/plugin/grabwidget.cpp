/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "grabwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

#include <KWindowSystem>

Q_DECLARE_METATYPE(QColor)

QDBusArgument &operator<<(QDBusArgument &argument, const QColor &color)
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
    Q_EMIT colorChanged();
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
    m_grabWidget->grabKeyboard();
}

bool X11Grabber::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease) {
        releaseWidget();

        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        if (me->button() == Qt::LeftButton) {
            const QPoint pos = me->globalPos();
            const QDesktopWidget *desktop = QApplication::desktop();
            const QPixmap pixmap = QGuiApplication::screens().at(desktop->screenNumber())->grabWindow(desktop->winId(), pos.x(), pos.y(), 1, 1);
            if (!pixmap.isNull()) {
                QImage i = pixmap.toImage();
                QColor color(i.pixel(0, 0));
                setColor(color);
            }
        }
    } else if (watched == m_grabWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent *me = static_cast<QKeyEvent *>(event);

        if (me->key() == Qt::Key_Escape) {
            releaseWidget();
        }
    }

    return QObject::eventFilter(watched, event);
}

void X11Grabber::releaseWidget()
{
    m_grabWidget->removeEventFilter(this);
    m_grabWidget->hide();
    m_grabWidget->releaseMouse();
    m_grabWidget->releaseKeyboard();
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
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        QDBusPendingReply<QColor> reply = *watcher;
        if (!reply.isError()) {
            setColor(reply.value());
        }
    });
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
