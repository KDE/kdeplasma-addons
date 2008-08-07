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

#ifndef LANCELOT_PARTS_H
#define LANCELOT_PARTS_H

#include <plasma/applet.h>
#include <lancelot/Global.h>
#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/ActionListView.h>

class LancelotPart : public Plasma::Applet
{
    //Q_OBJECT
public:
    LancelotPart(QObject * parent, const QVariantList &args);
    ~LancelotPart();

    void init();
    bool load(const QString & data);

    void saveConfig(const QString & data);
    bool loadConfig();
protected:

private:
    Lancelot::Instance * m_instance;
    Lancelot::ActionListView * m_list;
    Lancelot::ActionListViewModel * m_model;
    bool m_deleteModel;
    QGraphicsLayout * m_layout;
    QString m_cmdarg;

};

K_EXPORT_PLASMA_APPLET(lancelot-part, LancelotPart)

#endif
