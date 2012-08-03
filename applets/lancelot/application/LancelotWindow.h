/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTWINDOW_H
#define LANCELOTWINDOW_H

#include <QEvent>
#include <QSignalMapper>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <KIcon>
#include <KCompletion>

#include <Plasma/Svg>
#include <Plasma/Plasma>
#include <Plasma/Corona>

#include <Lancelot/PopupList>
#include <Lancelot/PopupMenu>

#include "LancelotConfig.h"
#include "ui_LancelotWindowBase.h"

namespace Lancelot {
    class ActionListModel;
    class ResizeBordersPanel;
}

namespace Plasma
{
    class LineEdit;
}

class LancelotWindow: public QGraphicsView, public Ui::LancelotWindowBase
{
    Q_OBJECT
public:
    LancelotWindow();
    virtual ~LancelotWindow();
    bool eventFilter(QObject * object, QEvent * event);

public Q_SLOTS:
    /**
     * Shows full window centered on the screen.
     */
    void lancelotShowCentered();

    /**
     * Shows full window centered on the screen.
     */
    void lancelotToggleCentered();

    /**
     * Shows full window.
     * @param x x coordinate of the mouse
     * @param y y coordinate of the mouse
     */
    void lancelotShow(int x, int y);

    /**
     * Shows window with only specified section
     * @param x x coordinate of the mouse
     * @param y y coordinate of the mouse
     * @param name the name of the section to be shown
     */
    void lancelotShowItem(int x, int y, const QString & name);

    /**
     * Hides window
     * @param immediate if not set, the window will start \
     *   the auto-timer for hiding. If true, window will be \
     *   hidden immediately
     */
    void lancelotHide(bool immediate = false);

    /**
     * Opens the search section and searches for
     * the specified string,
     * @param string search string
     */
    void search(const QString & string);

protected Q_SLOTS:
    /**
     * Shows specified section
     * @param item section to be shown
     */
    void sectionActivated(const QString & item);

private:
    void nextSection(int increment);

private Q_SLOTS:
    void systemButtonClicked();

    void lancelotContext();
    void saveConfig();
    void showAboutDialog();

    void hideImmediate();

    void focusChanged(QWidget * old, QWidget * now);
    void toggleCollapsedSections();
    void updateCollapsedSections(bool toggle = false);

public:
    /**
     * Returns a list of available sections - their IDs
     */
    QStringList sectionIDs();

    /**
     * Returns a list of available sections - their names
     */
    QStringList sectionNames();

    /**
     * Returns a list of available sections - their icons
     */
    QStringList sectionIcons();

    /**
     * @returns currently open section
     */
    QString currentSection();

public Q_SLOTS:
    /**
     * Opens the configure shortcuts dialog
     */
    void configureShortcuts();

    /**
     * Opens the configure menu dialog
     */
    void configureMenu();

    /**
     * Call this when an external application changes
     * the configuration
     */
    void configurationChanged();

    /**
     * Starts the menu editor
     */
    void showMenuEditor();

protected:
    /**
     * Resizes window and relayouts everything.
     * @returns if it was actually resized (false if
     * the old size is the new size)
     */
    bool updateWindowSize();

    /**
     * Shows window
     * @param x x coordinate of position to show at
     * @param y y coordinate of position to show at
     * @param centered override x and y and show window centered
     */
    void showWindow(int x, int y, bool centered = false);

    /**
     * Creates and sets up data models
     */
    void setupModels();

    /**
     * Loads configuration options
     */
    void loadConfig();

    /**
     * Sets up the actions
     */
    void setupActions();

    // void paintEvent(QPaintEvent * event);
    void drawBackground(QPainter * painter, const QRectF & rect);

private:
    Lancelot::ResizeBordersPanel * m_root;
    Plasma::Corona      * m_corona;

    Plasma::LineEdit    * editSearch;
    KCompletion         * m_completion;

    QHash < Lancelot::ExtenderButton *, QString > systemButtonActions;

    QTimer                m_hideTimer;
    QString               m_searchString;

    bool                  m_hovered;
    bool                  m_showingFull;

    QSignalMapper       * m_sectionsSignalMapper;
    KActionCollection   * m_actionCollection;

    QMap < QString, Lancelot::ActionListModel * > m_models;
    QMap < QString, Lancelot::ActionListModel * > m_modelGroups;

    KConfig               m_config;
    KConfigGroup          m_mainConfig;
    QString               m_activeSection;

    LancelotConfig m_configUi;
    QWidget * m_configWidget;

// Resize related code
protected:
    void mousePressEvent   (QMouseEvent * event);
    void mouseReleaseEvent (QMouseEvent * event);
    void mouseMoveEvent    (QMouseEvent * event);
    void resizeEvent       (QResizeEvent * event);

    void sendKeyEvent      (QKeyEvent * event);

private:
    enum BaseDirection {
        None = 0,
        Up   = 1, Down  = 2,
        Left = 4, Right = 8
    };

    Q_DECLARE_FLAGS(ResizeDirection, BaseDirection)

    ResizeDirection m_resizeDirection;
    QPoint          m_originalMousePosition;
    QPoint          m_originalWindowPosition;
    QSize           m_originalMainSize;
    QSize           m_mainSize;

    QList < QGraphicsWidget * > m_focusList;
    int m_focusIndex;
    bool m_skipEvent;

    // Additional widgets
    Lancelot::PopupList * menuSystemButton;
    Lancelot::PopupMenu * menuLancelotContext;

    // Caches
    QPoint          m_cachedOpenPosition;
    QSize           m_cachedWindowSize;
    Plasma::Flip    m_cachedFlip;
    bool            m_cachedOpenPositionCentered : 1;
    bool            m_cachedShowingFull : 1;
    bool            m_firstOpen : 1;
};

#endif /*LANCELOTWINDOW_H*/
