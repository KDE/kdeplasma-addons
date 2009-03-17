/* -*- C++ -*- */

/**********************************************************************/
/*   TimeMon (c)  1994  Helmut Maierhofer			      */
/*   KDE-ified M. Maierhofer 1998                                     */
/*   maintained by Dirk A. Mueller <mueller@kde.org                   */
/**********************************************************************/

/*
 * timemon.h
 *
 * Definitions for the timemon widget.
 */

#ifndef __TIMEMON_H__
#define __TIMEMON_H__

#include <qtooltip.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <kiconloader.h>
#include <kconfigdialog.h>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <QGraphicsSceneMouseEvent>
#include "ui_config.h"

namespace Plasma
{
}


// -- global constants ---------------------------------------------------

const int MAX_MOUSE_ACTIONS = 3; // event handlers for the three buttons only

// -- forward declaration ------------------------------------------------
class KSample;
class QPaintEvent;
class QMouseEvent;
class QPainter;
class K3Process;
class KProcess;
class KHelpMenu;
class KMenu;

// -- KTimeMon declaration -----------------------------------------------

/*
 * KTimeMon
 *
 * This is the main widget of the application. It handles the configuration
 * dialog and may have an associated KTimeMonWidget in the panel (in which
 * case it hides itself).
 */

class KTimeMon :  public Plasma::Applet
{
    Q_OBJECT
public:
    enum MouseAction { NOTHING=0, SWITCH=1, MENU=2, COMMAND=3 };

    KTimeMon(QObject *parent, const QVariantList &args);
    ~KTimeMon();

    void writeConfiguration();	// write back the configuration data

    // reimplemented from KPanelApplet
    int widthForHeight(int height) const;
    int heightForWidth(int width) const;
    void toggleScale(bool state);

    void stop();
    void cont();
    void init();

public slots:
    void timeout();		// timer expired
    void save();			// session management callback

protected:
    void updateTip();
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void updateConfig();
    void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    

private slots:			// called from the menu
    void orientation();		// switch vertical/horizontal orientation
    void commandStderr();
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted();
    void about();

private:
    void runCommand(int index);
    void paintRect(int x, int y, int w, int h, QColor c, QPainter *p);

    unsigned interval;
    bool autoScale;
    unsigned pageScale, swapScale, ctxScale;
    KMenu* menu;
    KHelpMenu* hmenu;
    QTimer* timer;
    MouseAction mouseAction[MAX_MOUSE_ACTIONS];
    QString mouseActionCommand[MAX_MOUSE_ACTIONS];
    KProcess *bgProcess;

    KSample *sample;
    QColor kernelColour, userColour, niceColour, iowaitColour;
    QColor usedColour, buffersColour, cachedColour, mkernelColour;
    QColor swapColour, bgColour;
    bool vertical, tooltip;

    KIcon *m_icon;
    QAction *action;

    Ui::config uiConfig;
};


#endif // TIMEMON_H
