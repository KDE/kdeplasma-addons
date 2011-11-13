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


#ifndef ABSTRACTTASKITEM_H
#define ABSTRACTTASKITEM_H

// KDE
#include <KColorScheme>
#include <KUrl>

// Own
#include "taskmanager/taskgroup.h"

// Qt
#include <QTime>
#include <QIcon>
#include <QFocusEvent>
#include <QGraphicsWidget>
#include <QPropertyAnimation>

class QTextOption;
class QTextLayout;
class QString;
class QAction;

// Plasma
#include <Plasma/Animator>

class Tasks;
class TaskGroupItem;
class LayoutWidget;
class DockItem;
class UnityItem;

/**
 * A baseclass for a task
 */
class AbstractTaskItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF animationPos READ animationPos WRITE setAnimationPos)
    Q_PROPERTY(qreal backgroundFadeAlpha READ backgroundFadeAlpha WRITE setBackgroundFadeAlpha)

public:
    enum InfoSource {
        IS_None,
        IS_Job,
        IS_DockManager,
        IS_Unity
    };

    enum Cache {
        Cache_Bgnd  = 0x01,
        Cache_Scale = 0x02,
        Cache_All   = Cache_Bgnd | Cache_Scale
    };

    static void clearCaches(int cache = Cache_All);

    /** Constructs a new representation for an abstract task. */
    AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet);

    /** Destruct the representation for an abstract task. */
    virtual ~AbstractTaskItem();

    /** The text changed for this task item. */
    void textChanged();

    /** Sets the icon for this task item. */
    void setIcon(const QIcon &icon);

    /**
     * This enum describes the generic flags which are currently
     * set by the task.
     */
    enum TaskFlag {
        /**
         * This flag is set by the task to indicate that it wants
         * the user's attention.
         */
        TaskWantsAttention = 1,
        /**
         * Indicates that the task's window has the focus
         */
        TaskHasFocus       = 2,
        /**
         * Indicates that the task is iconified
         */
        TaskIsMinimized    = 4
    };
    Q_DECLARE_FLAGS(TaskFlags, TaskFlag)

    /** Sets the task flags for this item. */
    void setTaskFlags(TaskFlags flags);

    /** Returns the task's current flags. */
    TaskFlags taskFlags() const;

    /** Returns current text for this task. */
    virtual QString text() const;

    /** Returns the current icon for this task. */
    QIcon icon(bool useDockManager = false) const;

    virtual void close() = 0;

    /** Tells the window manager the minimized task's geometry. */
    virtual void publishIconGeometry() const;
    virtual void publishIconGeometry(const QRect &rect) const;
    QRect iconGeometry() const; // helper for above

    /** Overridden from LayoutItem */
    void setGeometry(const QRectF& geometry);

    /** Convenience Functions to get information about Grouping */
    /** Only true if the task is not only member of rootGroup */
    bool isGrouped() const;
    bool isGroupMember(const TaskGroupItem *group) const;
    TaskGroupItem *parentGroup() const;

    virtual bool isWindowItem() const = 0;
    virtual bool isActive() const = 0;

    virtual void setAdditionalMimeData(QMimeData* mimeData) = 0;

    void setLayoutWidget(LayoutWidget* widget);
    TaskManager::AbstractGroupableItem * abstractItem();

    /** Returns the preferred size calculated on base of the fontsize and the iconsize*/
    QSize basicPreferredSize() const;
    void setPreferredOffscreenSize();
    void setPreferredOnscreenSize();

    //TODO: to be removed when we have proper animated layouts
    QPointF animationPos() const;
    void setAnimationPos(const QPointF &pos);

    virtual QGraphicsWidget *busyWidget() const {
        return 0L;
    }
    bool isStartupWithTask() const;
    bool isToolTipVisible() const;

    virtual void showContextMenu(const QPoint &, bool) { }
    virtual QString appName() const = 0;
    virtual KUrl launcherUrl() const = 0;
    virtual QString windowClass() const = 0;
    void updateProgress(int v, InfoSource source = IS_Job);
    void dockItemUpdated();
    void unityItemUpdated();
    void setDockItem(DockItem *i) {
        m_dockItem = i;
    }
    void setUnityItem(UnityItem *i) {
        m_unityItem = i;
    }
    virtual int pid() const {
        return 0;
    }
    virtual void toCurrentDesktop() { }
    QString mediaButtonKey();

Q_SIGNALS:
    void activated(AbstractTaskItem *);
    void destroyed(AbstractTaskItem *);

public Q_SLOTS:
    virtual void activate() = 0;
    void toolTipAboutToShow();
    void toolTipHidden();
    void mediaButtonPressed(int b);
    void windowPreviewActivated(WId id, Qt::MouseButtons buttons, Qt::KeyboardModifiers, const QPoint &pos);
    void controlWindow(WId id, Qt::MouseButtons buttons);

protected:
    void middleClick();
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

    // reimplemented
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void drawProgress(QPainter *painter, const QRectF &rect);
    void drawBadge(QPainter *painter, const QRectF &rect, const QString &badge);
    void drawIndicators(QPainter *painter, const QRectF &rect);
    void drawColoredBackground(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void drawShine(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void addOverlay(QPixmap &pix);

    /** Draws the background for the task item. */
    virtual void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option);
    /** Draws the icon and text which represent the task item. */
    virtual void drawTask(QPainter *painter, const QStyleOptionGraphicsItem *option, bool showText);

    virtual void updateTask(::TaskManager::TaskChanges changes) = 0; // pure virtual function
    virtual void updateToolTip() = 0; // pure virtual function
    void updateToolTipMediaState();
    void clearToolTip();
    void stopWindowHoverEffect();
    bool shouldIgnoreDragEvent(QGraphicsSceneDragDropEvent *event);
    QList<QAction *> getAppMenu();
    void registerWithHelpers();
    void unregisterFromHelpers();

protected Q_SLOTS:
    /** Event compression **/
    void queueUpdate();

    qreal backgroundFadeAlpha() const;
    void setBackgroundFadeAlpha(qreal progress);

    void syncActiveRect();
    void checkSettings();
    void clearAbstractItem();

protected:
    // area of item occupied by task's icon
    QRectF iconRect(const QRectF &bounds, bool showText = false);
    QSize iconSize(const QRectF &bounds) const;
    // area of item occupied by task's text
    QRectF textRect(const QRectF &bounds);
    // start an animation to chnge the task background
    void fadeBackground(const QString &newBackground, int duration);
    // text color, use this because it could be animated
    QColor textColor() const;
    void resizeBackground(const QSize &size);

    void resizeEvent(QGraphicsSceneResizeEvent *event);

    TaskManager::AbstractGroupableItem * m_abstractItem;
    LayoutWidget *m_layoutWidget;

    Tasks *m_applet;
    TaskFlags m_flags;

    // distance (in pixels) between a task's icon and its text
    static const int IconTextSpacing = 4;
    static const int TaskItemHorizontalMargin = 4;
    static const int TaskItemVerticalMargin = 4;

    //TODO: remove when we have animated layouts
    QPropertyAnimation *m_layoutAnimation;
    QPropertyAnimation *m_backgroundFadeAnim;

    qreal m_alpha;
    QString m_oldBackgroundPrefix;
    QString m_backgroundPrefix;
    DockItem *m_dockItem;
    UnityItem *m_unityItem;

private:
    QRectF m_activeRect;

    QTime m_lastGeometryUpdate;
    QTime m_lastUpdate;
    QSize m_lastIconSize;
    int m_activateTimerId;
    int m_updateGeometryTimerId;
    int m_updateTimerId;
    int m_hoverEffectTimerId;
    int m_attentionTimerId;
    int m_attentionTicks;
    int m_mediaStateTimerId;

    WId m_lastViewId;

    bool m_layoutAnimationLock : 1;
    bool m_firstGeometryUpdate : 1;

    mutable QIcon m_icon;

    InfoSource m_progressSource;
    int m_lastProgress;
    int m_currentProgress;
    QPointF m_oldDragPos;
};

#endif
