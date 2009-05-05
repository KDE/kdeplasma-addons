/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include <KConfigDialog>
#include <QBasicTimer>

#include <plasma/applet.h>
#include <plasma/popupapplet.h>

#include <lancelot/Global.h>
#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/ActionListView.h>

#include "PartsMergedModel.h"
#include "LancelotPartConfig.h"

class LancelotPart : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    LancelotPart(QObject * parent, const QVariantList &args);
    ~LancelotPart();

    void init();
    L_Override void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
    L_Override void dropEvent(QGraphicsSceneDragDropEvent * event);

    L_Override bool eventFilter(QObject * object, QEvent * event);

private Q_SLOTS:
    void configAccepted();

protected:
    L_Override void createConfigurationInterface(KConfigDialog *parent);
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);
    L_Override QGraphicsWidget * graphicsWidget();
    L_Override void timerEvent(QTimerEvent * event);

private Q_SLOTS:
    void removeModel(int index);

private:
    bool load(const QString & data);
    bool loadFromFile(const QString & url);
    bool loadFromList(const QStringList & list);
    bool loadDirectory(const QString & url);

    void saveConfig();
    bool loadConfig();
    void applyConfig();

    Lancelot::Instance * m_instance;
    Lancelot::ActionListView * m_list;
    Models::PartsMergedModel * m_model;
    QList < Lancelot::ActionListModel * > m_models;

    QString m_cmdarg;
    QString m_data;
    Plasma::IconWidget * m_icon;
    bool m_iconClickActivation;
    QBasicTimer m_timer;

    LancelotPartConfig m_config;
};

K_EXPORT_PLASMA_APPLET(lancelot-part, LancelotPart)

#endif // LANCELOT_PARTS_H
