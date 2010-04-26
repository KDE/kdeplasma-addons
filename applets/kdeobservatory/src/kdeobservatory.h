/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#ifndef KDEOBSERVATORY_HEADER
#define KDEOBSERVATORY_HEADER

#include <KConfigGroup>

#include <Plasma/PopupApplet>

class QTimer;
class QTimeLine;
class QNetworkReply;
class QGraphicsLinearLayout;

class ICollector;
class IViewProvider;
class KdeObservatoryConfigViews;
class KdeObservatoryConfigGeneral;
class KdeObservatoryConfigProjects;

namespace Plasma
{
    class Label;
    class Meter;
    class Engine;
    class PushButton;
}

class KdeObservatory : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    KdeObservatory(QObject *parent, const QVariantList &args);
    ~KdeObservatory();
    void init();
    
    virtual QGraphicsWidget* graphicsWidget();

    struct Project
    {
        QString commitSubject;
        QString krazyReport;
        QString krazyFilePrefix;
        QString icon;
    };

protected:
    bool eventFilter(QObject *receiver, QEvent *event);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

protected Q_SLOTS:
    void safeInit();
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();
    void collectFinished();
    void moveViewRight();
    void moveViewLeft();
    void moveViewRightClicked();
    void moveViewLeftClicked();
    void switchViews(int delta);
    void runCollectors();

private:
    void prepareUpdateViews();
    void updateViews();
    void loadConfig();
    void saveConfig();
    void createTimers();
    void createViewProviders();

    KConfigGroup m_configGroup;

    KdeObservatoryConfigGeneral *m_configGeneral;
    KdeObservatoryConfigProjects *m_configProjects;
    KdeObservatoryConfigViews *m_configViews;

    // Config - General
    int  m_commitExtent;
    int  m_synchronizationDelay;
    bool m_cacheContents;
    bool m_enableAutoViewChange;
    int  m_viewsDelay;
    int  m_lastViewCount;
    QList< QPair<QString, bool> > m_activeViews;

    // Config - Projects
    QMap<QString, Project> m_projects;

    // Config - Top Active Projects
    QHash<QString, bool> m_topActiveProjectsViewProjects;

    // Config - Top Developers
    QHash<QString, bool> m_topDevelopersViewProjects;

    // Config - Commit History
    QHash<QString, bool> m_commitHistoryViewProjects;

    // Config - Krazy
    QHash<QString, bool> m_krazyReportViewProjects;

    // Main Layout
    QGraphicsWidget *m_mainContainer;
    QGraphicsLinearLayout *m_horizontalLayout;
    QGraphicsWidget *m_viewContainer;
    Plasma::Meter *m_collectorProgress;
    Plasma::Label *m_updateLabel;
    Plasma::PushButton *m_right;
    Plasma::PushButton *m_left;

    // View providers
    QMap<QString, IViewProvider *> m_viewProviders;

    QList<QGraphicsWidget *> m_views;
    int m_currentView;

    QTimer *m_viewTransitionTimer;
    QTimer *m_synchronizationTimer;
    QTimeLine *m_transitionTimer;

    // Collectors
    QMap<QString, ICollector *> m_collectors;
    int m_collectorsFinished;
    
    Plasma::DataEngine *m_engine;
};

#endif
