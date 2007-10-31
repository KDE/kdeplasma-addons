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

#include <QtGui>
#include <QtCore>

#include "ui_LancelotWindow.h"
#include "ActionListView.h"

#include "models/Devices.h"
#include "models/Places.h"
#include "models/SystemServices.h"
#include "models/RecentDocuments.h"
#include "models/OpenDocuments.h"
#include "models/NewDocuments.h"
#include "models/FolderModel.h"

namespace Plasma {
    class Phase;
}

class LancelotWindow : public QFrame, public Ui::LancelotWindow
{
    Q_OBJECT
public:
	LancelotWindow( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~LancelotWindow();

    bool lancelotShow();
    bool lancelotHide(bool immediate = false);
    bool lancelotShowItem(QString name);

protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);
    void focusOutEvent(QFocusEvent * event);

private Q_SLOTS:
    void sectionActivated(const QString & item);
    void activated(int index);
    
    void systemLock();
    void systemLogout();
    void systemSwitchUser();

    void systemDoLock();
    void systemDoLogout();
    void systemDoSwitchUser();

private:
    void createModels();

    // Computer models
    Lancelot::MergedActionListViewModel * m_systemLeftModel;
    Lancelot::MergedActionListViewModel * m_systemRightModel;

    Lancelot::Models::Devices           * m_devicesModelRemovable;
    Lancelot::Models::Devices           * m_devicesModelFixed;
    Lancelot::Models::Places            * m_placesModel;
    Lancelot::Models::SystemServices    * m_systemServicesModel;

    // Documents models
    Lancelot::MergedActionListViewModel * m_documentsLeftModel;
    Lancelot::MergedActionListViewModel * m_documentsRightModel;

    Lancelot::Models::RecentDocuments   * m_recentDocumentsModel;
    Lancelot::Models::OpenDocuments     * m_openDocumentsModel;
    Lancelot::Models::NewDocuments      * m_newDocumentsModel;

    //Lancelot::Models::Devices           * m_devicesModelFixed;
    //Lancelot::Models::Places            * m_placesModel;
    //Lancelot::Models::SystemServices    * m_systemServicesModel;

    QTimer m_hideTimer;
    bool m_hovered;
    QSignalMapper * m_sectionsSignalMapper;
    Plasma::Phase * m_phase;

};

#endif /*LANCELOTWINDOW_H_*/
