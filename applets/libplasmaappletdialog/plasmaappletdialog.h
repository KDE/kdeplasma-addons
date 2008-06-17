/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef PLASMAAPPLETDIALOG_H
#define PLASMAAPPLETDIALOG_H

#include <plasma/applet.h>
#include <plasma/dialog.h>
#include "plasmaappletdialog_export.h"

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

namespace Plasma
{
    class Icon;
}

class PLASMAAPPLETDIALOG PlasmaAppletDialog : public Plasma::Applet
{
    Q_OBJECT
public:
    PlasmaAppletDialog(QObject *parent, const QVariantList &args);
    ~PlasmaAppletDialog();

    void init();

    virtual QWidget *widget() = 0;

    virtual void initMinimumSize();

private Q_SLOTS:
    void slotOpenDialog();

protected:
    virtual void initialize();
    void constraintsEvent(Plasma::Constraints constraints);
    QSize m_minimumSize;
    Plasma::Icon *m_icon;
    Plasma::Dialog *m_dialog;
    bool m_closePopup;

private:
    QGraphicsLinearLayout *m_layout;
    QGraphicsProxyWidget * m_proxy;
};

#endif /* PLASMAAPPLETDIALOG_H */

