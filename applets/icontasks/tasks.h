/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

#ifndef TASKS_H
#define TASKS_H

// Own
#include "ui_appearanceconfig.h"
#include "ui_behaviourconfig.h"

// Qt
#include <QWeakPointer>
#include <QTimer>
#include <QSize>

// KDE
#include "taskmanager/taskmanager.h"
#include "taskmanager/abstractgroupableitem.h"
#include "taskmanager/groupmanager.h"
#include "taskmanager/taskitem.h"
#include "taskmanager/startup.h"

// Plasma
#include <Plasma/Applet>

class QGraphicsLinearLayout;


namespace Plasma
{
class LayoutAnimator;
class FrameSvg;
} // namespace Plasma

namespace TaskManager
{
class GroupManager;
} // namespace TaskManager

class TaskGroupItem;
class GroupManager;

/**
 * An applet which provides a visual representation of running
 * graphical tasks (ie. tasks that have some form of visual interface),
 * and allows the user to perform various actions on those tasks such
 * as bringing them to the foreground, sending them to the background
 * or closing them.
 */
class Tasks : public Plasma::Applet
{
    Q_OBJECT
public:
    enum TT_Type {
        TT_None,
        TT_Instant,
        TT_Delayed
    };

    enum SeparatorType {
        Sep_Never,
        Sep_WhenNeeded,
        Sep_Always
    };

    enum MiddleClick {
        MC_NewInstance,
        MC_Close,
        MC_None,
        MC_MoveToCurrentDesktop
    };

    enum GroupClick {
        GC_MinMax,
        GC_PresentWindows,
        GC_Popup
    };

    enum Style {
        Style_Plasma,
        Style_IconTasks,
        Style_IconTasksColored
    };
    /**
        * Constructs a new tasks applet
        * With the specified parent.
        */
    explicit Tasks(QObject *parent, const QVariantList &args = QVariantList());
    ~Tasks();

    void init();

    void constraintsEvent(Plasma::Constraints constraints);

    Plasma::FrameSvg *itemBackground();
    Plasma::FrameSvg *progressBar();
    Plasma::FrameSvg *badgeBackground();
    Plasma::Svg* indicators();

    qreal itemLeftMargin() {
        return m_leftMargin;
    }
    qreal itemRightMargin() {
        return m_rightMargin;
    }
    qreal itemTopMargin() {
        return m_topMargin;
    }
    qreal itemBottomMargin() {
        return m_bottomMargin;
    }
    qreal offscreenLeftMargin() {
        return m_offscreenLeftMargin;
    }
    qreal offscreenRightMargin() {
        return m_offscreenRightMargin;
    }
    qreal offscreenTopMargin() {
        return m_offscreenTopMargin;
    }
    qreal offscreenBottomMargin() {
        return m_offscreenBottomMargin;
    }
    void resizeItemBackground(const QSizeF &newSize);
    void resizeProgressBar(const QSizeF &size);
    void resizeBadgeBackground(const QSizeF &size);

    TaskGroupItem *rootGroupItem();
    TaskManager::GroupManager &groupManager() const;

    bool showToolTip() const;
    bool instantToolTip() const;
    bool autoIconScaling() const;
    bool highlightWindows() const;
    bool launcherIcons() const {
        return m_launcherIcons;
    }
    GroupClick groupClick() const {
        return m_groupClick;
    }
    bool rotate() const {
        return m_rotate;
    }
    Style style() const {
        return m_style;
    }
    SeparatorType showSeparator() const {
        return m_showSeparator;
    }
    MiddleClick middleClick() const {
        return m_middleClick;
    }
    int spacing() const {
        return m_spacing;
    }
    int iconScale() const {
        return m_iconScale;
    }

    void needsVisualFocus(bool focus);
    QWidget *popupDialog() const;

    bool isPopupShowing() const;

    QList<QAction*> contextualActions();

signals:
    /**
        * emitted whenever we receive a constraintsEvent
        */
    void constraintsChanged(Plasma::Constraints);
    void settingsChanged();

public slots:
    void configChanged();
    void publishIconGeometry();

protected slots:
    void configAccepted();
    void setPopupDialog(bool status);

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

private slots:
    /**
    * Somthing has changed in the tree of the GroupingStrategy
    */
    void reload();
    void changeSizeHint(Qt::SizeHint which);
    void updateShowSeparator();
    void toolTipsModified();
    void styleModified();
    void refresh();
    void lockLaunchers();
    void unlockLaunchers();

private:
    TT_Type m_toolTips;
    bool m_highlightWindows;
    bool m_launcherIcons;
    GroupClick m_groupClick;
    bool m_rotate;
    Style m_style;
    SeparatorType m_showSeparator;
    MiddleClick m_middleClick;
    int m_spacing;
    int m_iconScale;
    Plasma::LayoutAnimator *m_animator;
    QGraphicsLinearLayout *layout;

    Ui::appearanceconfig m_appUi;
    Ui::behaviourconfig m_behaviourUi;
    QTimer m_screenTimer;

    Plasma::FrameSvg *m_taskItemBackground;
    Plasma::FrameSvg *m_progressBar;
    Plasma::FrameSvg *m_badgeBackground;
    Plasma::Svg *m_indicators;
    qreal m_leftMargin;
    qreal m_topMargin;
    qreal m_rightMargin;
    qreal m_bottomMargin;
    qreal m_offscreenLeftMargin;
    qreal m_offscreenTopMargin;
    qreal m_offscreenRightMargin;
    qreal m_offscreenBottomMargin;

    TaskGroupItem *m_rootGroupItem;
    GroupManager *m_groupManager;
    TaskManager::GroupManager::TaskGroupingStrategy m_groupingStrategy;
    bool m_groupWhenFull;

    int m_currentDesktop;
    QWeakPointer<QWidget> m_popupDialog;
    QAction *m_lockAct;
    QAction *m_unlockAct;
    QAction *m_refreshAct;
};

#endif
