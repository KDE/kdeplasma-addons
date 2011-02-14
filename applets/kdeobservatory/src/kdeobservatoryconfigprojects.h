/*************************************************************************
 * Copyright 2009-2010 Sandro Andrade sandroandrade@kde.org              *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#ifndef KDEOBSERVATORYCONFIGPROJECTS_HEADER
#define KDEOBSERVATORYCONFIGPROJECTS_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfigprojects.h"

class KdeObservatoryConfigProjects : public QWidget, public Ui::KdeObservatoryConfigProjects
{
    Q_OBJECT
public:
    explicit KdeObservatoryConfigProjects(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigProjects();

    void createTableWidgetItem(const QString &projectName, const QString &commitSubject, const QString &krazyReport, const QString &krazyFilePrefix, const QString &icon);

Q_SIGNALS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);

    // I've added the QString argument for feature possible use, don't know if it's really needed!
    void projectEdited(const QString &projectName);

public Q_SLOTS:
    void on_psbAddProject_clicked();
    void on_psbRemoveProject_clicked();
    void on_psbEditProject_clicked();
};

#endif
