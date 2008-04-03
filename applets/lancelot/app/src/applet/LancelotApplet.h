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
#include <plasma/layouts/nodelayout.h>

#include <lancelot/Global.h>
#include <lancelot/widgets/ExtenderButton.h>
#include "lancelot_interface.h"

#include "LancelotAppletConfig.h"

#include <QString>

class QDBusInterface;

class LancelotApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    LancelotApplet(QObject * parent, const QVariantList &args);
    ~LancelotApplet();

    QSizeF contentSizeHint() const;

    Qt::Orientations expandingDirections() const;

    bool hasHeightForWidth () const;
    qreal heightForWidth (qreal width) const;

    bool hasWidthForHeight () const;
    qreal widthForHeight (qreal height) const;

    void updateGeometry ();
    void constraintsUpdated (Plasma::Constraints constraints);

    void showConfigurationInterface();
    bool hasConfigurationInterface();

    void init();

protected Q_SLOTS:
    void showLancelot();
    void showLancelotSection(const QString & section);

    void configAccepted();

private:
    QSignalMapper m_signalMapper;

    org::kde::lancelot::App * m_lancelot;
    int m_clientID;

    Lancelot::Instance * m_instance;

    Plasma::NodeLayout * m_layout;
    QList< Lancelot::ExtenderButton * > m_buttons;

    LancelotAppletConfig * m_configDialog;

    // Current state / configuration
    bool m_isVertical;
    bool m_blockUpdates;

    bool m_showCategories;
    QString m_mainIcon;
    bool m_clickActivation;


    // Atoms
    void dbusConnect();

    void deleteButtons();
    void createCategories();
    void createMenuButton();
    void layoutButtons();

    void loadConfig();
    void saveConfig();
    void applyConfig();

};

K_EXPORT_PLASMA_APPLET(lancelot-applet, LancelotApplet)

#endif
