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

#include <QTimer>

class DraggableLabel : public Plasma::Label
{
public:
    explicit DraggableLabel(QGraphicsWidget *parent);
    KUrl m_url;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

class Pastebin : public Plasma::Applet
{
    Q_OBJECT
public:
    Pastebin(QObject *parent, const QVariantList &args);
    ~Pastebin();

    void init();
    void setTextServer(int backend);
    void setImageServer(int backend);

    enum textServers { PASTEBINCA, PASTEBINCOM };
    enum imageServers { IMAGEBINCA, IMAGESHACK };

public slots:
    void configAccepted();

protected slots:
    void showResults(const QString &url);
    void showErrors();
    void openLink(const QString &link);

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void createConfigurationInterface(KConfigDialog *parent);

private:
    DraggableLabel *m_resultsLabel;
    Plasma::Label *m_displayEdit;
    PastebinServer *m_textServer;
    PastebinServer *m_imageServer;

    int m_textBackend;
    int m_imageBackend;

    QTimer *timer;

    Ui::pastebinConfig uiConfig;
    Ui::pastebinServersConfig uiServers;
};

K_EXPORT_PLASMA_APPLET(pastebin, Pastebin)

#endif
