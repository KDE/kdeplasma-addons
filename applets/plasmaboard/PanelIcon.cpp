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
#include "Layout.h"
#include <QAction>
#include <QGraphicsView>

#include <KIcon>
#include <KWindowSystem>
#include <KStandardDirs>

#include <plasma/theme.h>
#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>


PanelIcon::PanelIcon(QObject *parent, const QVariantList &args)  :
        Plasma::PopupApplet(parent, args), m_plasmaboard(0){
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("preferences-desktop-keyboard");
    //setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setPassivePopup(true);
}


PanelIcon::~PanelIcon() {
	Plasma::ToolTipManager::self()->unregisterWidget(this);
    qDeleteAll(m_layouts);
}

QList<QAction*> PanelIcon::contextualActions(){
    QList<QAction*> list;
    Q_FOREACH(Layout* l, m_layouts){
        QAction *action = new QAction(l->name(), this);
        action->setData(l->path());
        connect(action, SIGNAL(triggered(bool)), this, SLOT(initKeyboard()));
        list << action;
    }

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    list << sep;

    return list;
}

void PanelIcon::init() {

    QStringList layoutList = KGlobal::dirs()->findAllResources("data", "plasmaboard/*.xml");
    Q_FOREACH(QString path, layoutList){
        m_layouts << new Layout(path);
    }

    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipContent toolTip;
    toolTip.setImage(KIcon("preferences-desktop-keyboard"));
    toolTip.setMainText(i18n("Virtual Keyboard"));
    Plasma::ToolTipManager::self()->setContent(this, toolTip);


    KConfigGroup cg = config();
    QString layout;
    layout = cg.readEntry("layout", layout);

    QString file = KStandardDirs::locate("data", layout); // lookup whether saved layout exists
    if(layout.size() > 0 && file.size() > 0){
        m_layout = file;
    }
    else{ // fallback to default layout
        m_layout = KStandardDirs::locate("data", "plasmaboard/qwert_layout.xml");
    }

}

void PanelIcon::initKeyboard() {
    QString path = ((QAction*)sender())->data().toString();
    qDebug() << path;
    m_plasmaboard->deleteKeys();
    m_plasmaboard->initKeyboard(path);
    m_plasmaboard->refreshKeys();
    m_plasmaboard->update();
    saveLayout(path);
}

void PanelIcon::initKeyboard(QString layoutFile) {
    m_plasmaboard->deleteKeys();
    m_plasmaboard->initKeyboard(layoutFile);
    saveLayout(layoutFile);
}

QGraphicsWidget *PanelIcon::graphicsWidget()
{
    if (!m_plasmaboard) {
        m_plasmaboard = new PlasmaboardWidget(this);
        initKeyboard(m_layout);
    }

    QGraphicsView *window = view();
    if (window) {
        KWindowInfo info = KWindowSystem::windowInfo(window->effectiveWinId(),  NET::WMWindowType);
        m_plasmaboard->setEnabled(info.windowType(NET::AllTypesMask) == NET::Dock);
    }

    return m_plasmaboard;
}

void PanelIcon::popupEvent(bool show){
	if ( !show ) {
        m_plasmaboard->reset();
	}
}

void PanelIcon::saveLayout(QString path) {

    int pos = path.indexOf("plasmaboard");

    KConfigGroup cg = config();
    cg.writeEntry("layout", path.right(path.size() - pos));

    emit configNeedsSaving();

}

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(plasmaboard, PanelIcon)
