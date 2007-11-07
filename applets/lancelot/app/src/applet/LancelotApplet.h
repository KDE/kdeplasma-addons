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
#include <plasma/widgets/nodelayout.h>

#include "../src/Global.h"
#include "../src/ExtenderButton.h"

#include <QString>

class QDBusInterface;

class LancelotApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    LancelotApplet(QObject * parent, const QVariantList &args);
    ~LancelotApplet();

    QSizeF contentSizeHint() const;
    void setGeometry(const QRectF & geometry);
    
    qreal heightForWidth(qreal width) const;

protected Q_SLOTS:
    void showLancelot();
    void showLancelotSection(const QString & section);

private:
    QDBusInterface * m_dbus;
    int m_clientID;

    Lancelot::Instance * m_instance;
    Lancelot::ExtenderButton * m_buttonMain;
    Plasma::NodeLayout * m_layout;
    
    QList< Lancelot::ExtenderButton * > m_sectionButtons;
    
};

K_EXPORT_PLASMA_APPLET(lancelot-applet, LancelotApplet)

#endif
