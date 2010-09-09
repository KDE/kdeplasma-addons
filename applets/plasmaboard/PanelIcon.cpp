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

#include <KConfigDialog>
#include <KIcon>
#include <KWindowSystem>
#include <KStandardDirs>

#include <plasma/theme.h>
#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>


PanelIcon::PanelIcon(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_plasmaboard(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("preferences-desktop-keyboard");
    //setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setPassivePopup(true);
    setHasConfigurationInterface(true);
}


PanelIcon::~PanelIcon()
{
    Plasma::ToolTipManager::self()->unregisterWidget(this);
    qDeleteAll(m_layouts);
}

void PanelIcon::configAccepted()
{
    initKeyboard(m_layout);
}

void PanelIcon::configChanged()
{
    KConfigGroup cg = config();
    QString layout;
    layout = cg.readEntry("layout", layout);

    QString old_layout = m_layout;
    QString file = KStandardDirs::locate("data", layout); // lookup whether saved layout exists
    if (layout.size() > 0 && file.size() > 0){
        m_layout = file;
    } else { // fallback to default layout
        m_layout = KStandardDirs::locate("data", "plasmaboard/full.xml");
    }
    if (m_plasmaboard && old_layout != m_layout) { // just rebuild the keyboard if the layout has actually changed
        initKeyboard(m_layout);
    }
}

void PanelIcon::layoutNameChanged(const QString &name)
{
    Layout *lay = m_layouts[0];

    Q_FOREACH(Layout* l, m_layouts){
        if(l->name() == name){
            lay = l;
            break;
        }
    }

    m_layout = lay->path();
    ui.descriptionLabel->setText(lay->description());
}

/*QList<QAction*> PanelIcon::contextualActions(){
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
}*/

void PanelIcon::createConfigurationInterface(KConfigDialog *parent)
{
    qDeleteAll(m_layouts);
    m_layouts.clear();
    QStringList layoutList = KGlobal::dirs()->findAllResources("data", "plasmaboard/*.xml");
    Q_FOREACH(QString path, layoutList){
        m_layouts << new Layout(path);
    }

    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18nc("Different keyboard layouts","Layouts"), "plasmaboard");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    Q_FOREACH(Layout* l, m_layouts){
        ui.layoutsComboBox->addItem(l->name(), l->path());
        if(l->path() == m_layout){
            ui.descriptionLabel->setText(l->description());
            ui.layoutsComboBox->setCurrentIndex(ui.layoutsComboBox->count() - 1);
        }
    }

    connect(ui.layoutsComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(layoutNameChanged(QString)));
}


void PanelIcon::init()
{
    configChanged();
}

void PanelIcon::initKeyboard()
{
    QString path = ((QAction*)sender())->data().toString();
    m_plasmaboard->deleteKeys();
    m_plasmaboard->initKeyboard(path);
    m_plasmaboard->refreshKeys();
    m_plasmaboard->update();
    saveLayout(path);
}

void PanelIcon::initKeyboard(const QString &layoutFile)
{
    m_plasmaboard->deleteKeys();
    m_plasmaboard->initKeyboard(layoutFile);
    m_plasmaboard->refreshKeys();
    m_plasmaboard->update();
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

void PanelIcon::popupEvent(bool show)
{
    if (!show) {
        m_plasmaboard->reset();
    }
}

void PanelIcon::saveLayout(const QString &path)
{
    int pos = path.indexOf("plasmaboard");

    KConfigGroup cg = config();
    cg.writeEntry("layout", path.right(path.size() - pos));

    emit configNeedsSaving();
}

void PanelIcon::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
            Plasma::ToolTipManager::self()->registerWidget(this);
            Plasma::ToolTipContent toolTip;
            toolTip.setImage(KIcon("preferences-desktop-keyboard"));
            toolTip.setMainText(i18n("Virtual Keyboard"));
            Plasma::ToolTipManager::self()->setContent(this, toolTip);
        } else {
            Plasma::ToolTipManager::self()->unregisterWidget(this);
        }
    }
}

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(plasmaboard, PanelIcon)
