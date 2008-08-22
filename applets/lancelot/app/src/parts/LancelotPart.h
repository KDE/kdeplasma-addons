/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef LANCELOT_PARTS_H
#define LANCELOT_PARTS_H

#include <plasma/applet.h>
#include <plasma/widgets/icon.h>
#include <plasma/dialog.h>
#include <KConfigDialog>
#include <lancelot/Global.h>
#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/ActionListView.h>
#include <lancelot/widgets/HoverIcon.h>
#include "PartsMergedModel.h"
#include "LancelotPartConfig.h"

class LancelotPart : public Plasma::Applet
{
    Q_OBJECT
public:
    LancelotPart(QObject * parent, const QVariantList &args);
    ~LancelotPart();

    void init();
    L_Override virtual void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
    L_Override virtual void dropEvent(QGraphicsSceneDragDropEvent * event);

    L_Override virtual bool eventFilter(QObject * object, QEvent * event);

private Q_SLOTS:
    void configAccepted();

protected:
    L_Override virtual void createConfigurationInterface(KConfigDialog *parent);
    L_Override virtual void constraintsEvent(Plasma::Constraints
            constraints);

private Q_SLOTS:
    void removeModel(int index);
    void iconActivated();

private:
    bool load(const QString & data);
    bool loadFromFile(const QString & url);
    bool loadFromList(const QStringList & list);
    bool loadDirectory(const QString & url);

    void saveConfig();
    bool loadConfig();
    void applyConfig();
    void setupAppletUi(bool force = false);
    void hidePopup();

    Lancelot::Instance * m_instance;
    Lancelot::ActionListView * m_list;
    Models::PartsMergedModel * m_model;
    QList < Lancelot::ActionListViewModel * > m_models;

    QGraphicsLayout * m_layout;
    QString m_cmdarg;
    QString m_data;

    Lancelot::HoverIcon * m_icon;
    Plasma::Dialog * m_dialog;
    QGraphicsView * m_widget;
    QGraphicsScene * m_scene;

    LancelotPartConfig m_config;
    bool m_wasConstrained : 1;
};

K_EXPORT_PLASMA_APPLET(lancelot-part, LancelotPart)

#endif // LANCELOT_PARTS_H
