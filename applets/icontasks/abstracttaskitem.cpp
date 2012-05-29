/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
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

// Own
#include "abstracttaskitem.h"
#include "dockitem.h"
#include "dockmanager.h"
#include "unity.h"
#include "jobmanager.h"
#include "mediabuttons.h"
#include "unityitem.h"
#include "recentdocuments.h"

// Qt
#include <QApplication>
#include <QFileInfo>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QTextLayout>
#include <QVarLengthArray>
#include <QPropertyAnimation>
#include <QCache>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

// KDE
#include <KAuthorized>
#include <KColorUtils>
#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconEffect>
#include <KIconLoader>
#include <KRun>
#include <KDesktopFile>

#include <NETWinInfo>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include "tooltips/tooltipmanager.h"
#include <Plasma/WindowEffects>

#include "taskmanager/task.h"
#include "taskmanager/taskmanager.h"
#include "taskmanager/taskgroup.h"

#include "tasks.h"
#include "taskgroupitem.h"
#include "applauncheritem.h"

#include <cmath>
#include <iterator>

struct Tile {
    QPixmap left;
    QPixmap center;
    QPixmap right;
};

static QCache<quint64, QColor> colorCache;
static QCache<quint64, Tile> tileCache(50);
static QCache<QString, QPixmap> scaledCache(50);
static QPixmap shineCache;

static QAction theSepAction("Separator", 0L);

void AbstractTaskItem::clearCaches(int cache)
{
    if (cache & Cache_Bgnd) {
        colorCache.clear();
        tileCache.clear();
        shineCache = QPixmap();
    }

    if (cache & Cache_Scale) {
        scaledCache.clear();
    }
}

static QPixmap scaleIcon(const QIcon &icon, const QSize &sz, const QPixmap &pix)
{
    static const int constStep = 4;
    QSize s((sz.width() / constStep)*constStep, (sz.height() / constStep)*constStep);
    QString key;
    key.sprintf("%llx-%x-%x", icon.cacheKey(), s.width(), s.height());
    if (scaledCache.contains(key)) {
        return *scaledCache[key];
    }

    QPixmap *scaled = new QPixmap(pix.scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    scaledCache.insert(key, scaled);
    return *scaled;
}

static bool hsvLess(const QColor &c1, const QColor &c2)
{
    int h1, s1, v1, h2, s2, v2;
    c1.getHsv(&h1, &s1, &v1);
    c2.getHsv(&h2, &s2, &v2);

    return
        (h1 << 16 | s1 << 8 | v1) <
        (h2 << 16 | s2 << 8 | v2);
}

static bool isNear(const QColor &c1, const QColor &c2)
{
    int h1, s1, v1, h2, s2, v2;
    c1.getHsv(&h1, &s1, &v1);
    c2.getHsv(&h2, &s2, &v2);

    return
        qAbs(h1 - h2) <=  8 &&
        qAbs(s1 - s2) <= 16 &&
        qAbs(v1 - v2) <= 32;
}

static QColor dominantColor(const QIcon &icon)
{
    if (colorCache.contains(icon.cacheKey())) {
        return *colorCache[icon.cacheKey()];
    }

    static const QColor constBlackSubstitute(64 , 64 , 64);
    static const int constMin = 32; // Min value of r, g, and b

    QImage          image(icon.pixmap(32, 32).toImage());
    QVector<QColor> colors(image.width() * image.height());

    int count = 0;

    // find the mean color
    for (int x = 0; x < image.width(); ++ x) {
        for (int y = 0; y < image.height(); ++ y) {
            QRgb rgb = image.pixel(x, y);

            // only use non-(total-)transparent colors
            if (qAlpha(rgb) != 0) {
                QColor color(rgb);

                // only use colors that aren't too grey
                if (color.saturation() > 24) {
                    colors[count] = color;

                    ++ count;
                }
            }
        }
    }

    if (count == 0) {
        colorCache.insert(icon.cacheKey(), new QColor(constBlackSubstitute));
        return *colorCache[icon.cacheKey()];
    }

    colors.resize(count);
    qSort(colors.begin(), colors.end(), hsvLess);

    int mid = count / 2;
    QColor midColor(colors[mid]);
    QColor *begin = colors.data() + mid;

    // find similar colors before the mean:
    if (mid != 0) {
        -- begin;

        while (begin != colors.data()) {
            if (isNear(*(begin - 1), midColor)) {
                -- begin;
            } else {
                break;
            }
        }
    }

    QColor* end = colors.data() + mid;

    // find similar colors after the mean:
    while (end != colors.data() + colors.size()) {
        if (isNear(*end, midColor)) {
            ++ end;
        } else {
            break;
        }
    }

    // average of similar colors:
    unsigned int r = 0, g = 0, b = 0;
    for (QColor* it = begin; it != end; ++ it) {
        r += it->red();
        g += it->green();
        b += it->blue();
    }

    int similarCount = std::distance(begin, end);
    QColor color(r / similarCount, g / similarCount, b / similarCount);
    int h, s, v;
    color.getHsv(&h, &s, &v);

    if (v < 196) {
        v = 196;
    } else if (v > 224) {
        v = 224;
    }

    if (s < 128) {
        s = 128;
    } else if (s > 196) {
        s = 196;
    }

    color.setHsv(h, s, v);

    static const int constStep = 8;
    QColor *col = new QColor((color.red() / constStep)*constStep, (color.green() / constStep)*constStep, (color.blue() / constStep)*constStep);

    if (col->red() < constMin && col->green() < constMin && col->blue() < constMin) {
        *col = constBlackSubstitute;
    }
    colorCache.insert(icon.cacheKey(), col);
    return *col;
}

const Tile & coloredBackground(const QColor &color, const QSize &size)
{
    qreal radius = qMin(4.0, size.width() / 4.0);
    int sectionWidth = qMax(2, (int)(radius + 1));

    quint64 key = (((quint64)(sectionWidth & 0xFFFF)) << 48) +
                  (((quint64)(size.height() & 0xFFFF)) << 32) +
                  (color.red() << 16) +
                  (color.blue() << 8) +
                  (color.green());
    if (tileCache.contains(key)) {
        return *tileCache[key];
    }

    QPixmap         pix(sectionWidth * 3, size.height());
    pix.fill(Qt::transparent);
    QPainter        painter(&pix);
    QPainterPath    path(Plasma::PaintUtils::roundedRectangle(QRectF(0.5, 0.5, pix.width() - 1, pix.height() - 1), radius));
    QLinearGradient grad(QPoint(0, 0), QPoint(0, pix.height()));
    QColor          col(color);

    col.setAlphaF(0.7);
    grad.setColorAt(0, KColorUtils::lighten(col, 0.5));
    grad.setColorAt(1.0, KColorUtils::darken(col, 0.35));

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(path, grad);

//     col.setAlphaF(0.5);
    painter.setPen(col); // KColorUtils::lighten(color, 0.2));
    painter.drawPath(path);
    painter.end();

    Tile *tile = new Tile;
    tile->left = pix.copy(0, 0, sectionWidth, pix.height());
    tile->center = pix.copy(sectionWidth, 0, sectionWidth, pix.height());
    tile->right = pix.copy(sectionWidth * 2, 0, sectionWidth, pix.height());
    tileCache.insert(key, tile);
    return *tile;
}

const QPixmap & shine(const QSize &sz)
{
    QSize size(sz);
    size.setHeight(sz.width() / 2);

    if (shineCache.size() == size) {
        return shineCache;
    }

    shineCache = QPixmap(size);
    shineCache.fill(Qt::transparent);
    QPainter        painter(&shineCache);
    QRadialGradient rad(QPointF(shineCache.width() / 2.0, 0), shineCache.width() / 2.0, QPointF(shineCache.width() / 2.0, 0));
    QColor          c(Qt::white);
    double          alpha(0.6);

    c.setAlphaF(alpha);
    rad.setColorAt(0, c);
    c.setAlphaF(alpha * 0.625);
//     rad.setColorAt(0.5, c);
//     c.setAlphaF(alpha*0.125);
//     rad.setColorAt(0.75, c);
    c.setAlphaF(0);
    rad.setColorAt(1, c);
    painter.fillRect(QRect(0, 1, shineCache.width(), shineCache.height()), rad);
    painter.end();

    return shineCache;
}

static QSize rotateFrame(const QSize &sz, bool rot)
{
    return rot ? QSize(sz.height(), sz.width()) : sz;
}

static const int HOVER_EFFECT_TIMEOUT = 900;

AbstractTaskItem::AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet)
    : QGraphicsWidget(parent),
      m_abstractItem(0),
      m_applet(applet),
      m_flags(0),
      m_backgroundFadeAnim(0),
      m_alpha(1),
      m_backgroundPrefix("normal"),
      m_dockItem(0),
      m_unityItem(0),
      m_activateTimerId(0),
      m_updateGeometryTimerId(0),
      m_updateTimerId(0),
      m_hoverEffectTimerId(0),
      m_attentionTimerId(0),
      m_attentionTicks(0),
      m_mediaStateTimerId(0),
      m_lastViewId(0),
//       m_showText(true),
      m_layoutAnimationLock(false),
      m_firstGeometryUpdate(false),
      m_progressSource(IS_None),
      m_lastProgress(-1),
      m_currentProgress(-1)
{
    m_layoutAnimation = new QPropertyAnimation(this, "animationPos", this);
    m_layoutAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_layoutAnimation->setDuration(250);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setFlag(QGraphicsItem::ItemIsFocusable);

    checkSettings();
    connect(applet->itemBackground(), SIGNAL(repaintNeeded()), this, SLOT(syncActiveRect()));
    connect(applet, SIGNAL(settingsChanged()), this, SLOT(checkSettings()));
    IconTasks::ToolTipManager::self()->registerWidget(this);
}

QSize AbstractTaskItem::basicPreferredSize() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    const int iconsize = KIconLoader::SizeSmall;

    int size = (int)qMin((mSize.width() * 12 + m_applet->itemLeftMargin() + m_applet->itemRightMargin() + iconsize),
                         qMax(mSize.height(), iconsize) + m_applet->itemTopMargin() + m_applet->itemBottomMargin());
    return QSize(size, size);
}

void AbstractTaskItem::setPreferredOffscreenSize()
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    int textWidth = fm.width(text());
    QSize mSize = fm.size(0, "M");
    int iconsize = KIconLoader::SizeSmall;

    QSizeF s(qMax(qMin(textWidth, 512) + 8, mSize.width() * 12) + m_applet->offscreenLeftMargin() + m_applet->offscreenRightMargin() + iconsize,
             qMax(mSize.height(), iconsize) + m_applet->offscreenTopMargin() + m_applet->offscreenBottomMargin());
    setPreferredSize(s);
}

void AbstractTaskItem::setPreferredOnscreenSize()
{
    setPreferredSize(basicPreferredSize());
}

AbstractTaskItem::~AbstractTaskItem()
{
    stopWindowHoverEffect();
    emit destroyed(this);
    IconTasks::ToolTipManager::self()->unregisterWidget(this);

    QList<int> timers = QList<int>() << m_activateTimerId
                        << m_updateGeometryTimerId
                        << m_updateTimerId
                        << m_hoverEffectTimerId
                        << m_attentionTimerId
                        << m_mediaStateTimerId;
    foreach (int t, timers) {
        if (t) {
            killTimer(t);
        }
    }
}

void AbstractTaskItem::checkSettings()
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem *>(this);

    if (m_applet->showToolTip() && (!group || group->collapsed())) {
        clearToolTip();
    } else {
        IconTasks::ToolTipManager::self()->unregisterWidget(this);
    }
}

void AbstractTaskItem::updateToolTipMediaState()
{
    IconTasks::ToolTipContent data;
    QString key = mediaButtonKey();

    if (!key.isEmpty()) {
        data.setPlayState(MediaButtons::self()->playbackStatus(key, pid()));
    }

    data.setAutohide(false);
    data.setMediaUpdate(true);
    IconTasks::ToolTipManager::self()->setContent(this, data);
}

void AbstractTaskItem::clearToolTip()
{
    if (m_mediaStateTimerId) {
        killTimer(m_mediaStateTimerId);
        m_mediaStateTimerId = 0;
    }
    IconTasks::ToolTipContent data;
#if KDE_IS_VERSION(4, 7, 0)
    data.setInstantPopup(m_applet->instantToolTip());
#endif

    IconTasks::ToolTipManager::self()->setContent(this, data);
}

void AbstractTaskItem::clearAbstractItem()
{
    m_abstractItem = 0;
}

QString AbstractTaskItem::text() const
{
    if (m_abstractItem) {
        return m_abstractItem->name();
    } else {
        kDebug() << "no abstract item?";
    }

    return QString();
}

QIcon AbstractTaskItem::icon(bool useDockManager) const
{
    if (useDockManager && m_dockItem && !m_dockItem->icon().isNull()) {
        return m_dockItem->icon();
    }

    if (m_abstractItem) {
        if (m_applet->launcherIcons() && m_icon.isNull()) {
            KUrl launcherUrl(m_abstractItem->launcherUrl());
            if (launcherUrl.isLocalFile() && KDesktopFile::isDesktopFile(launcherUrl.toLocalFile())) {
                KDesktopFile f(launcherUrl.toLocalFile());
                if (f.tryExec()) {
                    m_icon = KIcon(f.readIcon());
                }
            }
        }

        return m_applet->launcherIcons() && !m_icon.isNull() ? m_icon : m_abstractItem->icon();
    }

    return QIcon();
}

void AbstractTaskItem::setTaskFlags(TaskFlags flags)
{
    if ((flags & TaskWantsAttention) && (flags & TaskHasFocus)) {
        flags &= ~TaskWantsAttention;
    }

    if (((m_flags & TaskWantsAttention) != 0) != ((flags & TaskWantsAttention) != 0)) {
        //kDebug() << "task attention state changed" << m_attentionTimerId;
        m_flags = flags;
        if (flags & TaskWantsAttention) {
            m_applet->needsVisualFocus(true);
            // start attention getting
            if (!m_attentionTimerId) {
                m_attentionTimerId = startTimer(500);
            }
        } else {
            m_applet->needsVisualFocus(false);
            if (m_attentionTimerId) {
                killTimer(m_attentionTimerId);
                m_attentionTimerId = 0;
            }
        }
    }

    m_flags = flags;

    QString newBackground;
    if (m_flags & TaskIsMinimized) {
        newBackground = "minimized";
    } else if (m_flags & TaskHasFocus) {
        newBackground = "focus";
    } else {
        newBackground = "normal";
    }

    if (newBackground != m_backgroundPrefix) {
        fadeBackground(newBackground, 250);
    }
}

void AbstractTaskItem::fadeBackground(const QString &newBackground, int duration)
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem*>(this);
    if (group && !group->collapsed()) {
        return;
    }

    m_oldBackgroundPrefix = m_backgroundPrefix;
    m_backgroundPrefix = newBackground;

    if (m_oldBackgroundPrefix.isEmpty()) {
        update();
    } else {
        if (!m_backgroundFadeAnim) {
            m_backgroundFadeAnim = new QPropertyAnimation(this);
            m_backgroundFadeAnim->setDuration(duration);
            m_backgroundFadeAnim->setEasingCurve(QEasingCurve::InQuad);
            m_backgroundFadeAnim->setPropertyName("backgroundFadeAlpha");
            m_backgroundFadeAnim->setTargetObject(this);
            m_backgroundFadeAnim->setStartValue(0);
            m_backgroundFadeAnim->setEndValue(1);
        }

        m_backgroundFadeAnim->start();
    }
}

AbstractTaskItem::TaskFlags AbstractTaskItem::taskFlags() const
{
    return m_flags;
}

void AbstractTaskItem::toolTipAboutToShow()
{
    if (m_applet->showToolTip()) {
        updateToolTip();
        connect(IconTasks::ToolTipManager::self(),
                SIGNAL(windowPreviewActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)),
                this, SLOT(windowPreviewActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)));
        connect(IconTasks::ToolTipManager::self(),
                SIGNAL(windowButtonActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)),
                this, SLOT(controlWindow(WId, Qt::MouseButtons)));
        connect(IconTasks::ToolTipManager::self(),
                SIGNAL(mediaButtonPressed(int)), this, SLOT(mediaButtonPressed(int)));
    } else {
        clearToolTip();
    }
}

void AbstractTaskItem::toolTipHidden()
{
    clearToolTip();
    disconnect(IconTasks::ToolTipManager::self(),
               SIGNAL(windowPreviewActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)),
               this, SLOT(windowPreviewActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)));
    disconnect(IconTasks::ToolTipManager::self(),
               SIGNAL(windowButtonActivated(WId, Qt::MouseButtons, Qt::KeyboardModifiers, QPoint)),
               this, SLOT(controlWindow(WId, Qt::MouseButtons)));
    disconnect(IconTasks::ToolTipManager::self(),
               SIGNAL(mediaButtonPressed(int)), this, SLOT(mediaButtonPressed(int)));
}

QString AbstractTaskItem::mediaButtonKey()
{
    KUrl lUrl = launcherUrl();
    QString desktopEntry;
    if (lUrl.isValid()) {
        desktopEntry = lUrl.fileName().remove(".desktop").toLower();
        if (desktopEntry.startsWith("kde4-")) {
            desktopEntry = desktopEntry.mid(5);
        }
    }

    if (MediaButtons::self()->isMediaApp(desktopEntry)) {
        QString key = windowClass().toLower();

        if (key.isEmpty()) {
            key = desktopEntry;
        }

        return key;
    }

    return QString();
}

void AbstractTaskItem::mediaButtonPressed(int b)
{
    QString key = mediaButtonKey();
    if (!key.isEmpty()) {
        switch (b) {
        case IconTasks::ToolTipManager::MB_PREV:
            MediaButtons::self()->previous(key, pid());
            break;
        case IconTasks::ToolTipManager::MB_PLAY_PAUSE:
            MediaButtons::self()->playPause(key, pid());
            break;
        case IconTasks::ToolTipManager::MB_NEXT:
            MediaButtons::self()->next(key, pid());
            break;
        default:
            break;
        }

        // Update the playstate after a timer, to give player time to react...
        if (m_mediaStateTimerId) {
            killTimer(m_mediaStateTimerId);
            m_mediaStateTimerId = 0;
        }
        m_mediaStateTimerId = startTimer(250);
    }
}

void AbstractTaskItem::windowPreviewActivated(WId id, Qt::MouseButtons buttons, Qt::KeyboardModifiers, const QPoint &pos)
{
    if (buttons & Qt::LeftButton) {
        if (parentGroup()) {
            AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
            if (item) {
                IconTasks::ToolTipManager::self()->hide(this);
                item->activate();
            }
        }
    } else if (buttons & Qt::RightButton) {
        if (parentGroup()) {
            AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
            if (item) {
                // Onnly show menu if th e hide timer has not already gone off!!!
                if (IconTasks::ToolTipManager::self()->stopHideTimer(this)) {
                    item->showContextMenu(pos, false);
                    IconTasks::ToolTipManager::self()->startHideTimer(this);
                }
            }
        }
    } else if (buttons & Qt::MidButton) {
        switch (m_applet->middleClick()) {
        case Tasks::MC_Close: {
            if (parentGroup()) {
                AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
                if (item) {
                    item->m_abstractItem->close();
                }
            }
            break;
        }
        case Tasks::MC_MoveToCurrentDesktop: {
            if (parentGroup()) {
                AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
                if (item) {
                    item->toCurrentDesktop();
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void AbstractTaskItem::controlWindow(WId id, Qt::MouseButtons buttons)
{
    // TODO: More window actions...
    if (buttons & Qt::LeftButton) {
        if (parentGroup()) {
            AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
            if (item && item->m_abstractItem) {
                IconTasks::ToolTipManager::self()->hide(this);
                item->m_abstractItem->close();
            }
        }
    }
}

void AbstractTaskItem::middleClick()
{
    if (m_abstractItem) {
        switch (m_applet->middleClick()) {
        case Tasks::MC_NewInstance: {
            KUrl url = m_abstractItem->launcherUrl();

            if (!url.isEmpty()) {
                new KRun(url, 0);
            }
            break;
        }
        case Tasks::MC_Close:
            m_abstractItem->close();
            break;
        case Tasks::MC_MoveToCurrentDesktop:
            toCurrentDesktop();
            break;
        default:
            break;
        }
    }
}

void AbstractTaskItem::queueUpdate()
{
    if (m_updateTimerId || m_attentionTimerId) {
        return;
    }

    if (m_lastUpdate.elapsed() < 100) {
        m_updateTimerId = startTimer(100);
        return;
    }

    publishIconGeometry();
    update();
    m_lastUpdate.restart();
}

void AbstractTaskItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    setTaskFlags(m_flags | TaskHasFocus);
    update();
}

void AbstractTaskItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    setTaskFlags(m_flags & ~TaskHasFocus);
    update();
}

void AbstractTaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    fadeBackground("hover", 250);
    QGraphicsWidget *w = parentWidget();
    if (w && this != m_applet->rootGroupItem()) {
        if (m_hoverEffectTimerId) {
            killTimer(m_hoverEffectTimerId);
            m_hoverEffectTimerId = 0;
        }

        m_hoverEffectTimerId = startTimer(HOVER_EFFECT_TIMEOUT);
    }
}

void AbstractTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    stopWindowHoverEffect();

    QString backgroundPrefix;
    if (m_flags & TaskWantsAttention && 0 != m_attentionTimerId) {
        backgroundPrefix = "attention";
    } else if (m_flags & TaskIsMinimized) {
        backgroundPrefix = "minimized";
    } else if (m_flags & TaskHasFocus) {
        backgroundPrefix = "focus";
    } else {
        backgroundPrefix = "normal";
    }

    fadeBackground(backgroundPrefix, 150);
}

void AbstractTaskItem::stopWindowHoverEffect()
{
    if (m_hoverEffectTimerId) {
        killTimer(m_hoverEffectTimerId);
        m_hoverEffectTimerId = 0;
    }

    if (m_lastViewId && m_applet->highlightWindows()) {
        Plasma::WindowEffects::highlightWindows(m_lastViewId, QList<WId>());
    }
}

void AbstractTaskItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        if (boundingRect().contains(event->pos())) {
            activate();
        }
        break;
    case Qt::MidButton:
        middleClick();
        break;
    default:
        break;
    }
}

void AbstractTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    update();
}

void AbstractTaskItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug();
    if (QPoint(event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance()) {
        return;
    } //Wait a bit before starting drag

    QMimeData* mimeData = new QMimeData();
    //#ifndef ICON_TASKS_SHOW_DROP_INDICATOR_FOR_MOVE
    mimeData->setProperty("icontasks-item-ptr", (qulonglong)this);
    //#else
    //mimeData->setProperty("icontasks-item-index", m_applet->rootGroupItem()->indexOf(this, false));
    //#endif
    setAdditionalMimeData(mimeData);

    if (mimeData->formats().isEmpty()) {
        delete mimeData;
        return;
    }

    QDrag *drag = new QDrag(event->widget());
    drag->setMimeData(mimeData);
    //#ifndef ICON_TASKS_SHOW_DROP_INDICATOR_FOR_MOVE
    drag->setPixmap(icon().pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    //#else
    //drag->setPixmap(icon().pixmap(iconSize(m_applet->autoIconScaling() ? boundingRect().adjusted(4, 4, -5, -5) : boundingRect()).width()));
    //#endif
    drag->exec();
}

void AbstractTaskItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_activateTimerId) {
        killTimer(m_activateTimerId);
        m_activateTimerId = 0;
        if (!isActive()) {
            activate();
        }
    } else if (event->timerId() == m_updateGeometryTimerId) {
        killTimer(m_updateGeometryTimerId);
        m_updateGeometryTimerId = 0;
        m_firstGeometryUpdate = true;
        publishIconGeometry();
    } else if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        m_updateTimerId = 0;
        update();
    } else if (event->timerId() == m_attentionTimerId) {
        ++m_attentionTicks;
        if (m_attentionTicks > (Tasks::Style_Plasma == m_applet->style() ? 6 : 9)) {
            killTimer(m_attentionTimerId);
            m_attentionTimerId = 0;
            m_attentionTicks = 0;
        }

        if (m_attentionTicks % 2 == 0) {
            fadeBackground("attention", Tasks::Style_Plasma == m_applet->style() ? 200 : 300);
        } else {
            fadeBackground("normal", Tasks::Style_Plasma == m_applet->style() ? 250 : 375);
        }
    } else if (event->timerId() == m_hoverEffectTimerId) {
        killTimer(m_hoverEffectTimerId);
        m_hoverEffectTimerId = 0;
        if (!isUnderMouse()) {
            return;
        }

#ifdef Q_WS_X11
        QList<WId> windows;

        if (m_abstractItem && m_abstractItem->itemType() == TaskManager::GroupItemType) {
            TaskManager::TaskGroup *group = qobject_cast<TaskManager::TaskGroup *>(m_abstractItem);

            if (group) {
                TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(this);
                if (groupItem && groupItem->popupDialog()) {
                    kDebug() << "adding" << groupItem->popupDialog()->winId();
                    windows.append(groupItem->popupDialog()->winId());
                }

                foreach (AbstractGroupableItem * item, group->members()) {
                    if (item->itemType() == TaskManager::TaskItemType) {
                        TaskManager::TaskItem *taskItem = qobject_cast<TaskManager::TaskItem *>(item);
                        if (taskItem && taskItem->task()) {
                            windows.append(taskItem->task()->window());
                        }
                    } //TODO: if taskgroup, recurse through sub-groups?
                }
            }
        } else {
            WindowTaskItem *windowTaskItem = qobject_cast<WindowTaskItem *>(this);
            if (windowTaskItem && windowTaskItem->parent()) {
                TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(windowTaskItem->parent());
                if (groupItem && groupItem->popupDialog()) {
                    windows.append(groupItem->popupDialog()->winId());
                }
            }

            TaskManager::TaskItem *taskItem = qobject_cast<TaskManager::TaskItem *>(m_abstractItem);
            if (taskItem && taskItem->task()) {
                windows.append(taskItem->task()->window());
            }
        }

        stopWindowHoverEffect();
        QGraphicsView *view = m_applet->view();
        if (view && m_applet->highlightWindows()) {
            m_lastViewId = view->winId();
            Plasma::WindowEffects::highlightWindows(m_lastViewId, windows);
        }
#endif
    } else if (event->timerId() == m_mediaStateTimerId) {
        killTimer(m_mediaStateTimerId);
        m_mediaStateTimerId = 0;
        updateToolTipMediaState();
    } else {
        QGraphicsWidget::timerEvent(event);
    }
}

bool AbstractTaskItem::isStartupWithTask() const
{
    return busyWidget() && parentGroup() && parentGroup()->matchingItem(m_abstractItem);
}

bool AbstractTaskItem::isToolTipVisible() const
{
    return IconTasks::ToolTipManager::self()->isVisible(this);
}

void AbstractTaskItem::updateProgress(int v, InfoSource source)
{
    if (source == m_progressSource || IS_None == m_progressSource) {
        m_progressSource = source;
        m_currentProgress = v;
        if (m_lastProgress != m_currentProgress && (m_currentProgress < 1 || 100 == m_currentProgress || abs(m_currentProgress - m_lastProgress) > 2)) {
            queueUpdate();
        }

        if (m_currentProgress < 0) {
            m_progressSource = IS_None;
        }
    }
}

void AbstractTaskItem::dockItemUpdated()
{
    if (m_dockItem) {
        updateProgress(m_dockItem->progress(), IS_DockManager);
        queueUpdate();
    }
}

void AbstractTaskItem::unityItemUpdated()
{
    if (m_unityItem) {
        updateProgress(m_unityItem->progressVisible() ? m_unityItem->progress() : -1, IS_Unity);
        queueUpdate();
    }
}

void AbstractTaskItem::drawProgress(QPainter *painter, const QRectF &rect)
{
    if (rect.width() < 12 || rect.height() < 12) {
        return;
    }

    m_lastProgress = m_currentProgress;

    double height = qMin(8.0, rect.height() / 4.0);
    QRectF border(rect.x(), rect.bottom() - (height + 1), rect.width(), height);
    qreal fill = ((border.width() - 1.0) * m_currentProgress) / 100.0;
    Plasma::FrameSvg *svg = m_applet->progressBar();

    svg->setElementPrefix("bar-inactive");

    if (border.size() != svg->frameSize()) {
        m_applet->resizeProgressBar(border.size());
    }
    svg->paintFrame(painter, border.topLeft());

    if (fill > 0.0 && fill < 4.0) {
        fill = 4.0;
    }
    if (fill >= 2.0) {
        bool rtl = Qt::RightToLeft == layoutDirection();
        QRectF fillRect(rtl ? border.right() - fill : border.x(), border.y(), fill, border.height());
        svg->setElementPrefix("bar-active");

        if (fillRect.size() != svg->frameSize()) {
            m_applet->resizeProgressBar(fillRect.size());
        }

        svg->paintFrame(painter, fillRect.topLeft());
    }
}

void AbstractTaskItem::drawBadge(QPainter *painter, const QRectF &bounds, const QString &badge)
{
    static const qreal constMaxPercent = 0.95;
    static const int constBlockSize = 4;

    QFont font(KGlobalSettings::taskbarFont());
    font.setItalic(false);
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(badge).adjusted(-4, -2, 4, 2);
    int origWidth = textRect.width();

    // To save lots of re-sizing, adjust width in step of 4 pixels
    textRect.setWidth(((textRect.width() / constBlockSize)*constBlockSize) + (textRect.width() % constBlockSize ? constBlockSize : 0));

    if (textRect.width() > (bounds.width()*constMaxPercent)) {
        // Perhaps rounding has made it too big?
        textRect.setWidth(origWidth);
        if (textRect.width() > (bounds.width()*constMaxPercent)) {
            // Try decreasing font size...
            font = KGlobalSettings::smallestReadableFont();
            font.setItalic(false);
            fm = QFontMetrics(font);
            textRect = fm.boundingRect(badge).adjusted(-2, -1, 2, 1);
        }
    }

    if (textRect.width() <= (bounds.width()*constMaxPercent)) {
        QColor txtCol(textColor());
        bool rtl = Qt::RightToLeft == layoutDirection();
        QRectF textRectF(rtl ? bounds.x() : bounds.right() - textRect.width(),
                         bounds.y() + 1, textRect.width(), textRect.height());
        Plasma::FrameSvg *svg = m_applet->badgeBackground();

        if (Tasks::Style_Plasma != m_applet->style()) {
            textRectF.adjust(rtl ? -1 : 1, -1, rtl ? -1 : 1, -1);
        }

        svg->setElementPrefix(txtCol.value() > 160 ? "dark" : "light");
        if (textRectF.size() != svg->frameSize()) {
            m_applet->resizeBadgeBackground(textRectF.size());
        }
        svg->paintFrame(painter, textRectF.topLeft());
        painter->setFont(font);
        painter->setPen(QPen(txtCol, 1.0));
        painter->drawText(textRectF, Qt::AlignCenter, badge);
    }
}

void AbstractTaskItem::drawIndicators(QPainter *painter, const QRectF &bounds)
{
    QString suffix = m_flags & TaskWantsAttention ? "-attention" : m_flags & TaskIsMinimized ? "-minimized" : "";
    QString position;
    bool vertical = Plasma::Vertical == m_applet->formFactor();
    qreal dimension = vertical ? bounds.height() : bounds.width();
    qreal size = dimension > 48 ? qMin(24.0, dimension / 4.0) : qMin(12.0, dimension / 2.0);
    QSizeF iSize(vertical ? size * 0.75 : size, vertical ? size : size * 0.75);
    bool group = qobject_cast<TaskGroupItem *>(this);
    Plasma::Svg *svg = m_applet->indicators();

    if (iSize != svg->size()) {
        svg->resize(iSize);
    }

    switch (m_applet->location()) {
    case Plasma::TopEdge:
        if (group) {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - (iSize.width() * 1.5)) / 2.0) + (iSize.width() * 0.5),
                                        bounds.y()),
                       "down" + suffix);
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - (iSize.width() * 1.5)) / 2.0),
                                        bounds.y()),
                       "down" + suffix);
        } else {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - iSize.width()) / 2.0),
                                        bounds.y()),
                       "down" + suffix);
        }
        if (m_flags & TaskHasFocus) {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - iSize.width()) / 2.0),
                                        bounds.y() + (bounds.height() - iSize.height())),
                       "up" + suffix);
        }
        break;
    case Plasma::RightEdge:
        if (group) {
            svg->paint(painter, QPointF(bounds.x() + (bounds.width() - iSize.width()),
                                        bounds.y() + ((bounds.height() - (iSize.height() * 1.5)) / 2.0) + (iSize.height() * 0.5)),
                       "left" + suffix);
            svg->paint(painter, QPointF(bounds.x() + (bounds.width() - iSize.width()),
                                        bounds.y() + ((bounds.height() - (iSize.height() * 1.5)) / 2.0)),
                       "left" + suffix);
        } else {
            svg->paint(painter, QPointF(bounds.x() + (bounds.width() - iSize.width()),
                                        bounds.y() + ((bounds.height() - iSize.height()) / 2.0)),
                       "left" + suffix);
        }
        if (m_flags & TaskHasFocus) {
            svg->paint(painter, QPointF(bounds.x(),
                                        bounds.y() + ((bounds.height() - iSize.height()) / 2.0)),
                       "right" + suffix);
        }
        break;
    case Plasma::LeftEdge:
        if (group) {
            svg->paint(painter, QPointF(bounds.x(),
                                        bounds.y() + ((bounds.height() - (iSize.height() * 1.5)) / 2.0) + (iSize.height() * 0.5)),
                       "right" + suffix);
            svg->paint(painter, QPointF(bounds.x(),
                                        bounds.y() + ((bounds.height() - (iSize.height() * 1.5)) / 2.0)),
                       "right" + suffix);
        } else {
            svg->paint(painter, QPointF(bounds.x(),
                                        bounds.y() + ((bounds.height() - iSize.height()) / 2.0)),
                       "right" + suffix);
        }
        if (m_flags & TaskHasFocus) {
            svg->paint(painter, QPointF(bounds.x() + (bounds.width() - iSize.width()),
                                        bounds.y() + ((bounds.height() - iSize.height()) / 2.0)),
                       "left" + suffix);
        }
        break;
    default:
    case Plasma::BottomEdge:
        if (group) {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - (iSize.width() * 1.5)) / 2.0) + (iSize.width() * 0.5),
                                        bounds.y() + (bounds.height() - iSize.height())),
                       "up" + suffix);
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - (iSize.width() * 1.5)) / 2.0),
                                        bounds.y() + (bounds.height() - iSize.height())),
                       "up" + suffix);
        } else {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - iSize.width()) / 2.0),
                                        bounds.y() + (bounds.height() - iSize.height())),
                       "up" + suffix);
        }
        if (m_flags & TaskHasFocus) {
            svg->paint(painter, QPointF(bounds.x() + ((bounds.width() - iSize.width()) / 2.0),
                                        bounds.y()),
                       "down" + suffix);
        }
    }
}

void AbstractTaskItem::drawColoredBackground(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    QSize sz = size().toSize() - QSize(4, 4);
    const Tile &tile = coloredBackground(dominantColor(icon()), sz);
    QPointF pos = size().toSize() == m_activeRect.size().toSize() ? m_activeRect.topLeft() + QPoint(2, 2) : QPointF(2, 2);

    if (!tile.left.isNull()) {
        painter->drawPixmap(pos, tile.left);
        painter->drawTiledPixmap(pos.x() + tile.left.width(), pos.y(), sz.width() - (tile.left.width() + tile.right.width()), tile.center.height(), tile.center);
        painter->drawPixmap((pos.x() + sz.width()) - tile.right.width(), pos.y(), tile.right);
    }
}

void AbstractTaskItem::drawShine(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    QPixmap pixmap = shine(size().toSize() - QSize(4, 4));
    if (pixmap.size() == (m_activeRect.size().toSize() - QSize(4, 4))) {
        painter->drawPixmap(m_activeRect.topLeft() + QPoint(2, 2), pixmap);
    } else {
        painter->drawPixmap(QPoint(2, 2), pixmap);
    }
}

void AbstractTaskItem::addOverlay(QPixmap &pix)
{
    if (m_dockItem && !m_dockItem->overlayIcon().isNull()) {
        int overlaySize=(int)(qMin(16.0, qMin(pix.width(), pix.height())/3.0)+0.5);
        overlaySize=((overlaySize/4)*4)+(overlaySize%4 ? 4 : 0);
        if(overlaySize>4) {
            QPixmap overlay = m_dockItem->overlayIcon().pixmap(QSize(overlaySize, overlaySize));
            if(!overlay.isNull()) {
                QPainter overlayPainter(&pix);
                QPoint pos = Qt::RightToLeft == layoutDirection() ? QPoint(pix.width()-overlay.width()+1, 0) : QPoint(0, 0);
                overlayPainter.drawPixmap(pos, overlay);
            }
        }
    }
}

void AbstractTaskItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *)
{
    if (!m_abstractItem) {
        return;
    }

    qreal origOpacity = 1.0;
    bool fadeBackground = false;

    if (busyWidget()) {
        AbstractTaskItem *item = parentGroup()->matchingItem(m_abstractItem);
        if (item) {
            QRectF iconR = item->iconRect(/*m_applet->autoIconScaling() ? item->boundingRect().adjusted(4,4,-5,-5) : */item->boundingRect(), false);
            QPointF pos = item->mapToParent(QPointF(iconR.x(), iconR.y()));

            busyWidget()->setGeometry(QRectF(pos.x(), pos.y(), iconR.width(), iconR.height()));
            busyWidget()->show();
            setGeometry(QRectF(-1, -1, 1, 1)); // Hide this item...
            return;
        } else {
            origOpacity = painter->opacity();
            fadeBackground = true;
        }
    }

    //kDebug() << "painting" << (QObject*)this << text();
    painter->setRenderHint(QPainter::Antialiasing);
    QRectF bounds(boundingRect());
    bool showText = bounds.width() > (bounds.height() * 4); // Want text for popup!

    if ((Tasks::Style_Plasma == m_applet->style() || showText) && (m_abstractItem->itemType() != TaskManager::LauncherItemType)) { //Launchers have no frame
        // draw background
        drawBackground(painter, option);
        if (fadeBackground) {
            painter->setOpacity(origOpacity * 0.50);
        }
    } else if (Tasks::Style_IconTasksColored == m_applet->style()) {
        if (fadeBackground) {
            painter->setOpacity(origOpacity * 0.50);
        }
        drawColoredBackground(painter, option);
    }

    // draw icon and text
    drawTask(painter, option, showText);

    if (fadeBackground) {
        painter->setOpacity(origOpacity);
    }
}

void AbstractTaskItem::syncActiveRect()
{
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();
    itemBackground->setElementPrefix("normal");

    qreal left, top, right, bottom;
    itemBackground->getMargins(left, top, right, bottom);

    itemBackground->setElementPrefix("focus");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    itemBackground->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    m_activeRect = QRectF(QPointF(0, 0), size());
    m_activeRect.adjust(left - activeLeft, top - activeTop,
                        -(right - activeRight), -(bottom - activeBottom));

    itemBackground->setElementPrefix(m_backgroundPrefix);

    queueUpdate();
}

void AbstractTaskItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    syncActiveRect();
    resizeBackground(event->newSize().toSize());
}

void AbstractTaskItem::resizeBackground(const QSize &size)
{
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();
    bool             vertical = Plasma::Vertical == m_applet->formFactor(),
                     rot = vertical && m_applet->rotate();
    QSize            sz = rotateFrame(size, rot) + (vertical ? QSize(2, 2) : QSize(0, 0));

    itemBackground->setElementPrefix("focus");
    m_applet->resizeItemBackground(sz);
    itemBackground->setElementPrefix("normal");
    m_applet->resizeItemBackground(sz);
    itemBackground->setElementPrefix("minimized");
    m_applet->resizeItemBackground(sz);
    itemBackground->setElementPrefix("attention");
    m_applet->resizeItemBackground(sz);
    itemBackground->setElementPrefix("hover");
    m_applet->resizeItemBackground(sz);

    //restore the prefix
    itemBackground->setElementPrefix(m_backgroundPrefix);
}

void AbstractTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    bool rot = Plasma::Vertical == m_applet->formFactor() && m_applet->rotate();

    /*FIXME -could be done more elegant with caching in tasks in a qhash <size,svg>.
    -do not use size() directly because this introduces the blackline syndrome.
    -This line is only needed when we have different items in the taskbar because of an expanded group for example. otherwise the resizing in the resizeEvent is sufficient
    */
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();
    QPointF          adj = Plasma::LeftEdge == m_applet->location() || Plasma::RightEdge == m_applet->location()
                           ? QPointF(-1, -1) : QPointF(0, 0);

    if (rot) {
        painter->save();
        painter->rotate(-90);
        painter->translate(-boundingRect().height(), 0);
    }

    if (~option->state & QStyle::State_Sunken &&
            (!m_backgroundFadeAnim || m_backgroundFadeAnim->state() != QAbstractAnimation::Running)) {
        itemBackground->setElementPrefix(m_backgroundPrefix);
        //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
        if (rotateFrame(size().toSize(), rot) != itemBackground->frameSize()) {
            resizeBackground(size().toSize());
        }

        if (itemBackground->frameSize() == m_activeRect.size().toSize()) {
            itemBackground->paintFrame(painter, m_activeRect.topLeft() + adj);
        } else {
            itemBackground->paintFrame(painter, adj);
        }
        //itemBackground->paintFrame(painter, backgroundPosition);
        if (rot) painter->restore();
        return;
    }

    itemBackground->setElementPrefix(m_oldBackgroundPrefix);
    //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
    if (rotateFrame(size().toSize(), rot) != itemBackground->frameSize()) {
        resizeBackground(size().toSize());
    }

    QPixmap oldBackground;

    if (option->state & QStyle::State_Sunken) {
        oldBackground = QPixmap(m_activeRect.size().toSize());
        oldBackground.fill(Qt::transparent);
        m_alpha = 0.4;
    } else {
        oldBackground = itemBackground->framePixmap();
    }

    itemBackground->setElementPrefix(m_backgroundPrefix);
    //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
    if (rotateFrame(size().toSize(), rot) != itemBackground->frameSize()) {
        resizeBackground(size().toSize());
    }

    QPixmap result = Plasma::PaintUtils::transition(oldBackground, itemBackground->framePixmap(), m_alpha);

    if (result.size() == m_activeRect.size().toSize()) {
        painter->drawPixmap(m_activeRect.topLeft() + adj, result);
    } else {
        painter->drawPixmap(QPoint(0, 0) + adj, result);
    }
    if (rot) painter->restore();
}

void AbstractTaskItem::drawTask(QPainter *painter, const QStyleOptionGraphicsItem *option, bool showText)
{
    Q_UNUSED(option)

    QRectF boundOrig = boundingRect();
    QRectF bounds = boundOrig;

    if (/*(m_abstractItem->itemType() != TaskManager::LauncherItemType) &&*/ showText) {
        bounds = bounds.adjusted(m_applet->itemLeftMargin(), m_applet->itemTopMargin(), -m_applet->itemRightMargin(), -m_applet->itemBottomMargin());
    } else {
        bounds = bounds.adjusted(4, 4, -5, -5);
    }

    WindowTaskItem *window = qobject_cast<WindowTaskItem *>(this);
    QGraphicsWidget *busyWidget;
    busyWidget = window ? window->busyWidget() : 0;

    QRectF iconR = iconRect(m_applet->autoIconScaling() ? bounds : boundOrig, showText);

    if (busyWidget) {
        QRectF bwR = iconRect(boundOrig, false);
        QPointF pos = mapToParent(QPointF(bwR.x(), bwR.y()));
        busyWidget->setGeometry(QRectF(pos.x(), pos.y(), bwR.width(), bwR.height()));
        busyWidget->show();
    }

    /*
    kDebug() << bool(option->state & QStyle::State_MouseOver) << m_backgroundFadeAnim <<
        (m_backgroundFadeAnim ? m_backgroundFadeAnim->state() : QAbstractAnimation::Stopped);*/
    const bool fadingBg = m_backgroundFadeAnim && m_backgroundFadeAnim->state() == QAbstractAnimation::Running;
    QIcon icn(icon(true));
    QSize iSize = iconR.toRect().size();
    QPixmap result = icn.pixmap(iSize);

    if (!m_applet->autoIconScaling() && result.size() != iSize) {
        result = scaleIcon(icn, iconR.toRect().size(), result);
        if (result.size() != iSize) {
            int xmod = (iSize.width() - result.width()) / 2,
                ymod = (iSize.height() - result.height()) / 2;
            iconR.adjust(xmod, ymod, -xmod, -ymod);
        }
    }

    addOverlay(result);

    if ((!fadingBg && !(option->state & QStyle::State_MouseOver)) ||
            (m_oldBackgroundPrefix != "hover" && m_backgroundPrefix != "hover")) {
        // QIcon::paint does some alignment work and can lead to funny
        // things when icon().size() != iconR.toRect().size()
        qreal opacity = painter->opacity();
        if (Tasks::Style_Plasma != m_applet->style()) {
            if (m_attentionTimerId) {
                painter->setOpacity(0.25 + ((((m_alpha > 0.5 ? m_alpha : 1.0 - m_alpha) - 0.5) / 0.5) * 0.75));
            }
//             else if (m_flags & TaskIsMinimized) {
//                 painter->setOpacity(0.6*painter->opacity());
//             }
        }
        painter->drawPixmap(iconR.topLeft(), result);
        if (Tasks::Style_Plasma != m_applet->style() && (/* (m_flags & TaskIsMinimized) || */m_attentionTimerId)) {
            painter->setOpacity(opacity);
        }
    } else {
        KIconEffect *effect = KIconLoader::global()->iconEffect();

        if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState)) {
            if (qFuzzyCompare(qreal(1.0), m_alpha)) {
                result = effect->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
            } else {
                result = Plasma::PaintUtils::transition(result,
                                                        effect->apply(result, KIconLoader::Desktop,
                                                                KIconLoader::ActiveState), m_backgroundPrefix != "hover" ? 1 - m_alpha : m_alpha);
            }
        }
        painter->drawPixmap(iconR.topLeft(), result);
    }

    painter->setPen(QPen(textColor(), 1.0));

    if (m_abstractItem->itemType() != TaskManager::LauncherItemType) {
        if (showText) {
            QRect rect = textRect(bounds).toRect();
            if (rect.height() > 20) {
                rect.adjust(2, 2, -2, -2); // Create a text margin
            }

            QFont f(KGlobalSettings::taskbarFont());
            QFontMetrics fm(f);
            QString txt = fm.elidedText(text(), Qt::ElideRight, rect.width(), QPalette::WindowText);
            QColor txtCol(textColor());

            painter->setPen(txtCol);
            painter->setFont(font());
            if (txtCol.value() < 128 && rect.height() > 4) {
                int haloWidth = qMin(rect.width(), fm.width(txt));
                if (haloWidth > 4) {
                    Plasma::PaintUtils::drawHalo(painter, QRectF(rect.x() + 0.5, rect.y() + 0.5, haloWidth - 1, rect.height() - 1));
                }
            }
            painter->drawText(rect, txt, QTextOption(Qt::AlignVCenter));
            if (m_flags & TaskWantsAttention && 0 != m_attentionTimerId) {
                painter->drawText(rect.adjusted(1, 0, 1, 0), txt, QTextOption(Qt::AlignVCenter));
            }
        }

        TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(this);
        if (groupItem && Tasks::Style_Plasma == m_applet->style()) {
            int iSize = iconR.height() > 28 ? 16 : 8;
            QPixmap pix(KIcon("list-add").pixmap(iSize, iSize));
            bool    rtl = Qt::RightToLeft == layoutDirection();
            int     glow = 1,
                    x = (rtl ? iconR.left() : (iconR.right() - (pix.width()))) + 2,
                    y = (iconR.bottom() - (pix.height() + 2)) + 2;
            double  glowX = (x - glow) + ((pix.width() + (2 * glow)) / 2.0),
                    glowY = (y - glow) + ((pix.height() + (2 * glow)) / 2.0);

            QRadialGradient gradient(QPointF(glowX, glowY), (pix.width() + (2 * glow)) / 2.0, QPointF(glowX, glowY));
            QColor          c(Qt::white);
            c.setAlphaF(0.5);
            gradient.setColorAt(0, c);
            gradient.setColorAt(0.6, c);
            c.setAlphaF(0.0);
            gradient.setColorAt(1.0, c);
            painter->fillRect(QRect(x - glow, y - glow, pix.width() + (2 * glow), pix.height() + (2 * glow)), gradient);
            painter->drawPixmap(x, y, pix);
        }
    }

    if (showText) {
        return;
    }

    if (Tasks::Style_IconTasksColored == m_applet->style()) {
        drawShine(painter, option);
    }

    if (busyWidget) {
        return;
    }

    if (m_unityItem && m_unityItem->countVisible()) {
        drawBadge(painter, iconR, QString().setNum(m_unityItem->count()));
    } else if (m_dockItem && !m_dockItem->badge().isEmpty()) {
        drawBadge(painter, iconR, m_dockItem->badge());
    }

    if (!showText && JobManager::self()->isEnabled() && m_currentProgress >= 0) {
        drawProgress(painter, iconR);
    }

    if (Tasks::Style_Plasma != m_applet->style() && (m_abstractItem->itemType() != TaskManager::LauncherItemType)) {
        drawIndicators(painter, rect());
    }
}

qreal AbstractTaskItem::backgroundFadeAlpha() const
{
    return m_alpha;
}

void AbstractTaskItem::setBackgroundFadeAlpha(qreal progress)
{
    m_alpha = progress;
    update();
}

bool AbstractTaskItem::shouldIgnoreDragEvent(QGraphicsSceneDragDropEvent *event)
{
    bool locked = m_applet->groupManager().launchersLocked();

    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype()) ||
            event->mimeData()->hasFormat(TaskManager::Task::groupMimetype()) ||
            (!locked && event->mimeData()->hasFormat(AppLauncherItem::mimetype()))) {
        return true;
    }

    if (!locked && event->mimeData()->hasFormat("text/uri-list")) {
        // we want to check if we have .desktop files; if so, then we treat it as a possible
        // drop for a launcher
        const KUrl::List uris = KUrl::List::fromMimeData(event->mimeData());
        if (!uris.isEmpty()) {
            foreach (const QUrl & uri, uris) {
                KUrl url(uri);
                if (!url.isLocalFile()) {
                    return false;
                }

                const QString path = url.toLocalFile();

                if (QFileInfo(path).isDir()) {
                    return false;
                }

                if (KDesktopFile::isDesktopFile(path)) {
                    KDesktopFile f(path);
                    if (f.tryExec()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

QList<QAction *> AbstractTaskItem::getAppMenu()
{
    QList<QAction*> appMenu;
    bool addedDocs = false;
    bool addedUnityItems = false;
    KUrl lUrl = launcherUrl();

    if (lUrl.isValid()) {
        appMenu = RecentDocuments::self()->get(lUrl.fileName().remove(".desktop"));
        addedDocs = true;
    }

    if (m_unityItem) {
        QList<QAction *> unityActions = m_unityItem->menu();
        addedUnityItems = !unityActions.isEmpty();
        if (addedDocs && addedUnityItems) {
            theSepAction.setSeparator(true);
            appMenu.append(&theSepAction);
        }
        appMenu.append(unityActions);
    }

    if (m_dockItem && !addedUnityItems) {
        QList<QAction *> dockActions = m_dockItem->menu();
        if (addedDocs && !dockActions.isEmpty()) {
            theSepAction.setSeparator(true);
            appMenu.append(&theSepAction);
        }
        appMenu.append(dockActions);
    }

    return appMenu;
}

void AbstractTaskItem::registerWithHelpers()
{
    JobManager::self()->registerTask(this);
    DockManager::self()->registerTask(this);
    Unity::self()->registerTask(this);
}

void AbstractTaskItem::unregisterFromHelpers()
{
    JobManager::self()->unregisterTask(this);
    DockManager::self()->unregisterTask(this);
    Unity::self()->unregisterTask(this);
    m_dockItem = 0;
    m_unityItem = 0;
}

void AbstractTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (shouldIgnoreDragEvent(event)) {
        event->ignore();
        return;
    }

    event->accept();

    if (!m_activateTimerId) {
        m_activateTimerId = startTimer(250);
        m_oldDragPos = event->pos();
    }
}

void AbstractTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    // restart the timer so that activate() is only called after the mouse
    // stops moving
    if (m_activateTimerId && event->pos() != m_oldDragPos) {
        m_oldDragPos = event->pos();
        killTimer(m_activateTimerId);
        m_activateTimerId = startTimer(250);
    }
}

void AbstractTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    if (m_activateTimerId) {
        killTimer(m_activateTimerId);
        m_activateTimerId = 0;
    }
}

QRect AbstractTaskItem::iconGeometry() const
{
    if (!scene() || !boundingRect().isValid()) {
        return QRect();
    }

    QGraphicsView *parentView = 0;
    QGraphicsView *possibleParentView = 0;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView * view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
                view->sceneRect().contains(scenePos())) {
            if (view->isActiveWindow()) {
                parentView = view;
                break;
            } else {
                possibleParentView = view;
            }
        }
    }

    if (!parentView) {
        parentView = possibleParentView;

        if (!parentView) {
            return QRect();
        }
    }

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    return rect;
}

void AbstractTaskItem::publishIconGeometry() const
{
}

void AbstractTaskItem::publishIconGeometry(const QRect &rect) const
{
    Q_UNUSED(rect)
}

void AbstractTaskItem::setAnimationPos(const QPointF &pos)
{
    m_layoutAnimationLock = true;
    setPos(pos);
    m_layoutAnimationLock = false;
}

QPointF AbstractTaskItem::animationPos() const
{
    return pos();
}

void AbstractTaskItem::setGeometry(const QRectF& geometry)
{
    if (geometry == QGraphicsWidget::geometry()) {
        // Stop layout animiation! Without this a gap in the taskbar can appear when
        // a whole task group is closed (e.g. start 2 KCalcs (with a launcher), and
        // close via right-click menu).
        // Looks like we have a first animation to move item below KCalc group
        // down/right by 1 space he we immediately get a setGeometry moving back to
        // the start. But because geometry==QGraphicsWidget::geometry, nothing happend.
        m_layoutAnimation->stop();
        if (m_updateGeometryTimerId) {
            killTimer(m_updateGeometryTimerId);
            m_updateGeometryTimerId = 0;
        }
        return;
    }

    QPointF oldPos = pos();

    if (m_lastGeometryUpdate.elapsed() < 500) {
        if (m_updateGeometryTimerId) {
            killTimer(m_updateGeometryTimerId);
            m_updateGeometryTimerId = 0;
        }

        m_updateGeometryTimerId = startTimer(500 - m_lastGeometryUpdate.elapsed());
    } else {
        publishIconGeometry();
        m_lastGeometryUpdate.restart();
    }

    //TODO:remove when we will have proper animated layouts
    if (m_firstGeometryUpdate && !m_layoutAnimationLock) {
        QRectF animStartGeom(oldPos, geometry.size());
        QGraphicsWidget::setGeometry(animStartGeom);

        if (m_layoutAnimation->state() == QAbstractAnimation::Running) {
            m_layoutAnimation->stop();
        }

        m_layoutAnimation->setEndValue(geometry.topLeft());
        m_layoutAnimation->start();
    } else {
        QGraphicsWidget::setGeometry(geometry);
    }
}

QRectF AbstractTaskItem::iconRect(const QRectF &b, bool showText)
{
    QRectF bounds(b);
    const int right = bounds.right();

    if (showText) {
        //leave enough space for the text. useful in vertical panel
        bounds.setWidth(qMax(bounds.width() / 3, qMin(minimumSize().height(), bounds.width())));

        //restore right position if the layout is RTL
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            bounds.moveRight(right);
        }
    }

    m_lastIconSize = iconSize(bounds);

    if (showText) {
        return QStyle::alignedRect(QApplication::layoutDirection(),
                                   (showText ? Qt::AlignLeft : Qt::AlignCenter) | Qt::AlignVCenter,
                                   m_lastIconSize, bounds.toRect());
    } else {
        return QRectF(bounds.x() + ((bounds.width() - m_lastIconSize.width()) / 2.0),
                      bounds.y() + ((bounds.height() - m_lastIconSize.height()) / 2.0),
                      m_lastIconSize.width(), m_lastIconSize.height());
    }
}

QSize AbstractTaskItem::iconSize(const QRectF &bounds) const
{
    QSize size;
    if (m_applet->autoIconScaling()) {
        size = icon().actualSize(bounds.size().toSize());

        static const int constMargin = 2;

        if (size.width() == size.height()) {
            if (size.width() > KIconLoader::SizeSmall - constMargin && size.width() < KIconLoader::SizeSmall + constMargin) {
                size = QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            } else if (size.width() > KIconLoader::SizeSmallMedium - constMargin && size.width() < KIconLoader::SizeSmallMedium + constMargin) {
                size = QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
            } else if (size.width() > KIconLoader::SizeMedium - constMargin && size.width() < KIconLoader::SizeMedium + constMargin) {
                size = QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
            } else if (size.width() > KIconLoader::SizeLarge - constMargin && size.width() < KIconLoader::SizeLarge + constMargin) {
                size = QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
            } else if (size.width() > KIconLoader::SizeHuge - constMargin && size.width() < KIconLoader::SizeHuge + constMargin) {
                size = QSize(KIconLoader::SizeHuge, KIconLoader::SizeHuge);
            }
        }
    } else {
        size = bounds.size().toSize(); //
        int sz = (qMin(size.width(), size.height()) * m_applet->iconScale()) / 100;
        size = QSize(sz, sz);
    }

    return size;
}

QRectF AbstractTaskItem::textRect(const QRectF &bounds)
{
    QSize size(bounds.size().toSize());
    QRectF effectiveBounds(bounds);

    size.rwidth() -= int(iconRect(bounds, true).width()) + qMax(0, IconTextSpacing - 2);
    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter, size, effectiveBounds.toRect());
}

QColor AbstractTaskItem::textColor() const
{
    QColor color;
    qreal bias;
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    if ((m_oldBackgroundPrefix == "attention" || m_backgroundPrefix == "attention") &&
            m_applet->itemBackground()->hasElement("hint-attention-button-color")) {
        bool animatingBg = m_backgroundFadeAnim && m_backgroundFadeAnim->state() == QAbstractAnimation::Running;
        if (animatingBg) {
            if (m_oldBackgroundPrefix == "attention") {
                bias = 1 - m_alpha;
            } else {
                bias = m_alpha;
            }

            color = KColorUtils::mix(theme->color(Plasma::Theme::TextColor),
                                     theme->color(Plasma::Theme::ButtonTextColor), bias);
        } else if (m_backgroundPrefix != "attention") {
            color = theme->color(Plasma::Theme::TextColor);
        } else {
            color = theme->color(Plasma::Theme::ButtonTextColor);
        }
    } else {
        color = theme->color(Plasma::Theme::TextColor);
    }

    if (m_flags & TaskIsMinimized) {
        color.setAlphaF(0.85);
    }

    return color;
}

bool AbstractTaskItem::isGroupMember(const TaskGroupItem *group) const
{
    if (!m_abstractItem || !group) {
        kDebug() << "no task";
        return false;
    }

    return m_abstractItem->isGroupMember(group->group());

}

bool AbstractTaskItem::isGrouped() const
{
    if (!m_abstractItem) {
        kDebug() << "no item";
        return false;
    }

    return m_abstractItem->isGrouped();
}

TaskGroupItem * AbstractTaskItem::parentGroup() const
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem*>(parentWidget());

    //lucky case: directly in a group
    if (group) {
        return group;
    }

    //in a popup or a popup's popup?
    QObject *candidate = parentWidget();

    while (candidate) {
        group = qobject_cast<TaskGroupItem*>(candidate);
        candidate = candidate->parent();
        if (group) {
            return group;
        }
    }

    return 0;
}

TaskManager::AbstractGroupableItem * AbstractTaskItem::abstractItem()
{
    return m_abstractItem;
}

#include "abstracttaskitem.moc"
