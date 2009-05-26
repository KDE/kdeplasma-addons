/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
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

#ifndef PASTEBIN_H
#define PASTEBIN_H

#include "backends/backends.h"
#include "ui_pastebinConfig.h"
#include "ui_pastebinServersConfig.h"

#include <KDE/KIO/TransferJob>
#include <KDE/KIO/Job>

#include <Plasma/Applet>
#include <Plasma/Label>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

#include <QTimer>
#include <QPen>

class QSignalMapper;
class KAction;

class Pastebin : public Plasma::Applet
{
    Q_OBJECT
public:
    Pastebin(QObject *parent, const QVariantList &args);
    ~Pastebin();

    void init();
    QList<QAction*> contextualActions();
    void setTextServer(int backend);
    void setImageServer(int backend);
    void setHistorySize(int max);

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        const QRect &contents);
    void constraintsEvent(Plasma::Constraints constraints);

    enum textServers { PASTEBINCA, PASTEBINCOM };
    enum imageServers { IMAGEBINCA, IMAGESHACK };

    enum InteractionState { /* What is the user doing, used for visual feedback on user actions */
        Off = 0,            /* Not set */
        Waiting = 1,        /* Applet hanging around idle */
        Hovered = 2,        /* "empty" mouse over effect */
        Rejected = 3,       /* unsuitable content is dragged over us */
        DraggedOver = 5     /* suitable content is dragged over us */
    };
    //Q_DECLARE_FLAGS(InteractionStates, InteractionState)

    enum ActionState {   /* What is the applet doing */
        Unset = 0,       /* Not set */
        Idle = 1,        /* The applet has been started but nothing done yet */
        IdleError = 2,   /* The last action went wrong, but we're ready to give it another try */
        IdleSuccess = 4, /* Last action succeeded */
        Sending = 8      /* Sending data to the server, waiting for reply */
    };
    //Q_DECLARE_FLAGS(ActionStates, ActionState)

public slots:
    void configAccepted();

protected slots:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void showResults(const QString &url);
    void showErrors();
    void openLink();
    void postClipboard();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void createConfigurationInterface(KConfigDialog *parent);
    void paintPixmap(QPainter *painter, QPixmap &pixmap,
                     const QRectF &rect, qreal opacity = 1.0);

private slots:
    void animationUpdate(qreal progress);
    void updateTheme();
    void resetActionState();
    void copyToClipboard(const QString &url);

private:
    int iconSize();
    void showOverlay(bool show);
    void postContent(QString text, QImage imageData);

    void addToHistory(const QString &url);

    void setInteractionState(InteractionState state);
    void setActionState(ActionState state);

    ActionState m_actionState;
    InteractionState m_interactionState;

    bool m_isHovered;
    bool m_fadeIn;
    int m_animId;
    qreal m_alpha;

    QFont m_font;
    QPen m_linePen;
    QColor m_fgColor;
    QColor m_bgColor;
    KIcon* m_icon;

    Plasma::ToolTipContent toolTipData;
    PastebinServer *m_textServer;
    PastebinServer *m_imageServer;
    QString m_url;

    int m_textBackend;
    int m_imageBackend;

    QTimer *timer;

    int m_historySize;
    QSignalMapper *m_signalMapper;
    QList<QAction*> m_contextualActions;
    QList<QAction*> m_actionHistory;
    KAction *m_paste;
    QAction *m_topSeparator;
    QAction *m_bottomSeparator;

    Ui::pastebinConfig uiConfig;
    Ui::pastebinServersConfig uiServers;
};

K_EXPORT_PLASMA_APPLET(pastebin, Pastebin)

#endif
