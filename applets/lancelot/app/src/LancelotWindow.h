/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTWINDOW_H_
#define LANCELOTWINDOW_H_

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

#include "ui_LancelotWindowBase.h"

class CustomGraphicsView;

namespace Lancelot {
    class ActionListViewModel;
}

class LancelotWindow: public QWidget, public Ui::LancelotWindowBase
{
    Q_OBJECT
public:
    LancelotWindow();
    virtual ~LancelotWindow();

public Q_SLOTS:
    void lancelotShow(int x, int y);
    void lancelotShowItem(int x, int y, QString name);
    void lancelotHide(bool immediate = false);
    void search(const QString & string);

protected Q_SLOTS:
    void sectionActivated(const QString & item);

    void doSearch();

    void systemLock();
    void systemLogout();
    void systemSwitchUser();

    void systemDoLock();
    void systemDoLogout();
    void systemDoSwitchUser();

public:
    QStringList sectionIDs();
    QStringList sectionNames();
    QStringList sectionIcons();
    void focusOutEvent(QFocusEvent * event);

protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);
    void resizeWindow(QSize newSize);
    void showWindow(int x, int y, int width, int height);
    void setupModels();

private:
    Plasma::Widget      * m_root;
    CustomGraphicsView  * m_view;
    Plasma::Corona      * m_corona;
    QVBoxLayout         * m_layout;

    QTimer                m_hideTimer;
    QTimer                m_searchTimer;
    QString               m_searchString;

    bool                  m_hovered;
    QSignalMapper       * m_sectionsSignalMapper;

    QMap < QString, Lancelot::ActionListViewModel * > m_models;
    QMap < QString, Lancelot::ActionListViewModel * > m_modelGroups;

    Lancelot::Instance  * instance;
};

#endif /*LANCELOTWINDOW_H_*/
