/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef EXPLORERWINDOW_H
#define EXPLORERWINDOW_H

#include <QWidget>

#include <Plasma/Plasma>

class QBoxLayout;
class QGraphicsWidget;

namespace Plasma
{
    class Containment;
    class Corona;
    class FrameSvg;
    class WidgetExplorer;
}

class GroupExplorer;

class ExplorerWindow : public QWidget
{
    Q_OBJECT

public:
    static ExplorerWindow *instance();
    ~ExplorerWindow();

    virtual void setContainment(Plasma::Containment *containment);
    Plasma::Containment *containment() const;

    virtual void setLocation(const Plasma::Location &loc);
    Plasma::Location location() const;
    Qt::Orientation orientation() const;
    QPoint positionForPanelGeometry(const QRect &panelGeom) const;

    void showGroupExplorer();
    bool isControllerViewVisible() const;

    Plasma::FrameSvg *background() const;

protected:
    void setGraphicsWidget(QGraphicsWidget *widget);

    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void onActiveWindowChanged(WId id);
    void backgroundChanged();
    void adjustSize(int screen);
    void syncToGraphicsWidget();

private:
    ExplorerWindow(QWidget* parent = 0);

    static ExplorerWindow *s_instance;
    Plasma::Location m_location;
    QBoxLayout *m_layout;
    Plasma::FrameSvg *m_background;
    QWeakPointer<Plasma::Containment> m_containment;
    Plasma::Corona *m_corona;
    QGraphicsView *m_view;
    GroupExplorer *m_groupManager;
    QGraphicsWidget *m_graphicsWidget;
    QTimer *m_adjustViewTimer;
};

#endif

