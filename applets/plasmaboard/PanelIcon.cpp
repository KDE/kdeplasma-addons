/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PanelIcon.h"
#include <QAction>
#include <QGraphicsView>

#include <KIcon>
#include <KWindowSystem>

#include <plasma/theme.h>
#include <plasma/corona.h>
#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>


PanelIcon::PanelIcon(QObject *parent, const QVariantList &args)  :
	Plasma::PopupApplet(parent, args), m_plasmaboard(0){
        setAspectRatioMode(Plasma::IgnoreAspectRatio);
        setPopupIcon("preferences-desktop-keyboard");
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setPassivePopup(true);

        contextExtended = new QAction(i18n("Switch to basic mode"), this);
        connect(contextExtended, SIGNAL(triggered(bool)), this, SLOT(toggleMode()));
        contextBasic = new QAction(i18n("Switch to extended mode"), this);
        connect(contextBasic, SIGNAL(triggered(bool)), this, SLOT(toggleMode()));

	connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(initKeyboard()));

}


PanelIcon::~PanelIcon() {
	Plasma::ToolTipManager::self()->unregisterWidget(this);

        delete contextExtended;
        delete contextBasic;
}

void PanelIcon::init() {
        KConfigGroup cg = config();
        extendedMode = cg.readEntry("extendedMode", false);

        Plasma::ToolTipManager::self()->registerWidget(this);
        Plasma::ToolTipContent toolTip;
        toolTip.setImage(KIcon("preferences-desktop-keyboard"));
        toolTip.setMainText(i18n("Virtual Keyboard"));
        Plasma::ToolTipManager::self()->setContent(this, toolTip);
}

QGraphicsWidget *PanelIcon::graphicsWidget()
{
    if (!m_plasmaboard) {
        m_plasmaboard = new PlasmaboardWidget(this);
        initKeyboard();
    }

    QGraphicsView *window = view();
    if (window) {
        KWindowInfo info = KWindowSystem::windowInfo(window->effectiveWinId(),  NET::WMWindowType);
        m_plasmaboard->setEnabled(info.windowType(NET::AllTypesMask) == NET::Dock);
    }

    return m_plasmaboard;
}

QList<QAction*> PanelIcon::contextualActions(){
	QList<QAction*> list;
        list.append(extendedMode ? contextExtended : contextBasic);
	return list;
}

void PanelIcon::toggleMode(){
	KConfigGroup cg = config();
	extendedMode = !extendedMode;
	cg.writeEntry("extendedMode", extendedMode);
	initKeyboard();
}

void PanelIcon::initKeyboard() {
	m_plasmaboard->resetKeyboard();
	if (extendedMode) {
	    m_plasmaboard->initExtendedKeyboard();
	    m_plasmaboard->switcher->setText("<a href=\"#test\" style=\"color:"+Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name()+";\">"+i18n("Fewer Keys")+"</a>");
	}
	else{
	    m_plasmaboard->switcher->setText("<a href=\"#test\" style=\"color:"+Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name()+";\">"+i18n("More Keys")+"</a>");
	    m_plasmaboard->initBasicKeyboard();
	}
}

void PanelIcon::popupEvent(bool show){
	if ( !show ) {
		m_plasmaboard->clearAnything();
	}
}


// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(plasmaboard, PanelIcon)
