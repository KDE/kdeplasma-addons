/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <QBasicTimer>
#include <QGraphicsLinearLayout>

#include <KConfigDialog>
#include <KCompletion>

#include <Plasma/Applet>
#include <Plasma/PopupApplet>
#include <Plasma/LineEdit>

#include <Lancelot/Global>
#include <Lancelot/Widget>
#include <Lancelot/ActionListView>
#include <Lancelot/FullBorderLayout>

#include <Lancelot/Models/Runner>

#include "PartsMergedModel.h"
#include "LancelotPartConfig.h"

class IconOverlay;

class LancelotPart : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    LancelotPart(QObject * parent, const QVariantList &args);
    ~LancelotPart();

    void init();
    L_Override void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
    L_Override void dropEvent(QGraphicsSceneDragDropEvent * event);
    L_Override void setGeometry(const QRectF & rect);

public Q_SLOTS:
    void configChanged();
    void activated();

    void togglePopup();
    void setPopupVisible(bool show = true);
    void toolTipAboutToShow();

private Q_SLOTS:
    void configAccepted();
    void search(const QString & query);
    void fixFocus();

protected:
    L_Override void createConfigurationInterface(KConfigDialog *parent);
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);
    L_Override QGraphicsWidget * graphicsWidget();
    L_Override void timerEvent(QTimerEvent * event);
    L_Override bool eventFilter(QObject * object, QEvent * event);

private Q_SLOTS:
    void removeModel(int index);
    void immutabilityChanged(Plasma::ImmutabilityType value);
    void modelContentsUpdated();
    void resetSearch();
    void listSizeChanged();

    void updateShowingSize();
    void updateOverlay();

private:
    bool loadFromList(const QStringList & list);
    void showSearchBox(bool value);
    void updateIcon();

    void saveConfig();
    void loadConfig();
    void applyConfig();

    QGraphicsWidget * m_root;
    QGraphicsLinearLayout * m_layout;
    Lancelot::ActionListView * m_list;
    Plasma::LineEdit * m_searchText;

    Models::PartsMergedModel * m_model;
    Lancelot::Models::Runner * m_runnnerModel;

    QString m_cmdarg;
    Plasma::IconWidget * m_icon;
    IconOverlay * m_iconOverlay;
    bool m_iconClickActivation;
    QBasicTimer m_timer;
    int m_rootHeight;

    KCompletion * m_completion;

    LancelotPartConfig m_config;
};

K_EXPORT_PLASMA_APPLET(lancelot-part, LancelotPart)

#endif // LANCELOT_PARTS_H
