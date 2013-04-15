/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *                         Artur Duque de Souza <asouza@kde.org>           *
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

#include "ui_pastebinConfig.h"

#include <KDE/KIO/TransferJob>
#include <KDE/KIO/Job>

#include <Plasma/Applet>
#include <Plasma/Label>

#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

#include <QClipboard>
#include <QTimer>
#include <QPen>
#include <QWeakPointer>

class QSignalMapper;
class KAction;
class QPropertyAnimation;

namespace Plasma
{
    class Service;
}

namespace KNS3 {
    class DownloadDialog;
}

class Pastebin : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(qreal animationUpdate READ animationValue WRITE animationUpdate)
public:
    Pastebin(QObject *parent, const QVariantList &args);
    ~Pastebin();

    void init();
    QList<QAction*> contextualActions();
    void setHistorySize(int max);

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        const QRect &contents);
    void constraintsEvent(Plasma::Constraints constraints);

    qreal animationValue() const;

    enum InteractionState { /* What is the user doing, used for visual feedback on user actions */
        Off = 0,            /* Not set */
        Waiting = 1,        /* Applet hanging around idle */
        Hovered = 2,        /* "empty" mouse over effect */
        Rejected = 3,       /* unsuitable content is dragged over us */
        DraggedOver = 5     /* suitable content is dragged over us */
    };

    enum ActionState {   /* What is the applet doing */
        Unset = 0,       /* Not set */
        Idle = 1,        /* The applet has been started but nothing done yet */
        IdleError = 2,   /* The last action went wrong, but we're ready to give it another try */
        IdleSuccess = 4, /* Last action succeeded */
        Sending = 8      /* Sending data to the server, waiting for reply */
    };

    enum textServers { PASTEBINCA, PASTEBINCOM };
    enum imageServers { IMAGEBINCA, IMAGESHACK, SIMPLESTIMAGEHOSTING, IMGUR };

public slots:
    void configAccepted();
    void configChanged();
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void sourceAdded(const QString &source);
    void sourceRemoved(const QString &source);

protected slots:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void showResults(const QString &url);
    void showErrors();
    void openLink(bool old = true);
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
    void postingFinished(KJob *job);
    void getNewStuff();
    void newStuffFinished();
    void refreshConfigDialog();

private:
    int iconSize();
    void showOverlay(bool show);
    void postContent(QString text, const QImage& imageData);
    void postClipboard(bool preferSelection);

    void addToHistory(const QString &url);

    void setInteractionState(InteractionState state);
    void setActionState(ActionState state);

    void saveHistory();

    ActionState m_actionState;
    InteractionState m_interactionState;

    bool m_isHovered;
    bool m_fadeIn;
    qreal m_alpha;

    QTimer *m_timer;
    QWeakPointer<QPropertyAnimation> m_animation;

    QFont m_font;
    QPen m_linePen;
    QColor m_fgColor;
    QColor m_bgColor;

    Plasma::ToolTipContent toolTipData;
    QString m_url;
    QString m_oldUrl;

    QSignalMapper *m_signalMapper;
    QList<QAction*> m_contextualActions;
    QList<QAction*> m_actionHistory;
    KAction *m_paste;
    QAction *m_topSeparator;
    QAction *m_bottomSeparator;

    // New version - below here vars after refactor
    int m_historySize;

    QHash<KJob *, QString> m_pendingTempFileJobs;
    QHash<QString, QString> m_txtServers;
    QHash<QString, QString> m_imgServers;
    Plasma::DataEngine *m_engine;
    Plasma::Service *m_postingService;
    Ui::pastebinConfig uiConfig;
    KNS3::DownloadDialog* m_newStuffDialog;

    QClipboard::Mode lastMode;
};

K_EXPORT_PLASMA_APPLET(pastebin, Pastebin)

#endif
