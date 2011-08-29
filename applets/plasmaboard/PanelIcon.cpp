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
#include <QFile>
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
      m_plasmaboard(0),
      m_tempLayout(false)
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
    QString layout = config().readEntry("layout", QString());
    if (layout.isEmpty()) {
        // fallback to default layout
        layout = KStandardDirs::locate("data", "plasmaboard/full.xml");
    } else {
        // lookup whether saved layout exists
        const QString file = KStandardDirs::locate("data", layout); 
        if (!file.isEmpty()) {
            layout = file;
        }
    }

    // only rebuild the keyboard if the layout has actually changed
    if (layout != m_layout && QFile::exists(layout)) {
        m_layout = layout;
        initKeyboard(m_layout);
    }
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

void PanelIcon::createConfigurationInterface(KConfigDialog *parent)
{
    qDeleteAll(m_layouts);
    m_layouts.clear();
    QStringList layoutList = KGlobal::dirs()->findAllResources("data", "plasmaboard/*.xml");
    foreach (QString path, layoutList) {
        m_layouts << new Layout(path);
    }

    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18nc("Different keyboard layouts", "Layouts"), "plasmaboard");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));


    foreach (Layout * l, m_layouts) {
        ui.layoutsComboBox->addItem(l->name(), l->path());
        if (l->path() == m_layout) {
            ui.descriptionLabel->setText(l->description());
            ui.layoutsComboBox->setCurrentIndex(ui.layoutsComboBox->count() - 1);
        }
    }

    connect(ui.layoutsComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(layoutNameChanged(QString)));
    connect(ui.layoutsComboBox, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
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

void PanelIcon::layoutNameChanged(const QString &name)
{
    Layout *lay = m_layouts[0];

    foreach (Layout * l, m_layouts) {
        if (l->name() == name) {
            lay = l;
            break;
        }
    }

    m_layout = lay->path();
    ui.descriptionLabel->setText(lay->description());
}

void PanelIcon::init()
{
    configChanged();
}

void PanelIcon::initKeyboard()
{
    if (!m_plasmaboard) {
        return;
    }

    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }

    QString path = action->data().toString();
    setLayout(path);
    saveLayout(path);
}

void PanelIcon::initKeyboard(const QString &layoutFile)
{
    if (!m_plasmaboard) {
        return;
    }

    setLayout(layoutFile);
    saveLayout(layoutFile);
}

void PanelIcon::resetLayout()
{
    if (m_tempLayout) {
        setLayout(m_layout);
    }
}

void PanelIcon::showLayout(const QString &layout)
{
    kDebug() << layout;
    if (layout.isEmpty()) {
        resetLayout();
        return;
    }

    const QString file = KStandardDirs::locate("data", "plasmaboard/" + layout);
    if (!file.isEmpty()) {
        setLayout(file);
    } else if (QFile::exists(layout)) {
        initKeyboard(layout);
    }
}

void PanelIcon::setLayout(const QString &layoutFile)
{
    m_tempLayout = layoutFile != m_layout;
    m_plasmaboard->deleteKeys();
    m_plasmaboard->initKeyboard(layoutFile);
    m_plasmaboard->refreshKeys();
    m_plasmaboard->update();
}

void PanelIcon::popupEvent(bool show)
{
    if (!show) {
        m_plasmaboard->reset();
        resetLayout();
    }
}

void PanelIcon::saveLayout(const QString &path)
{
    int pos = path.indexOf("plasmaboard");

    KConfigGroup cg = config();
    cg.writeEntry("layout", path.right(path.size() - pos));

    emit configNeedsSaving();
}

#include "PanelIcon.moc"

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(plasmaboard, PanelIcon)

