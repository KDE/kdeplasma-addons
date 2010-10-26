/***************************************************************************
 *   Copyright (C) 2007 by Jesper Thomschutz <jesperht@yahoo.com>          *
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

#ifndef FIFTEENPUZZLE_H
#define FIFTEENPUZZLE_H

#include <plasma/popupapplet.h>
#include <Plasma/Label>
#include <Plasma/PushButton>

#include <QGraphicsWidget>
#include <QTimer>

#include "fifteen.h"
#include "ui_fifteenPuzzleConfig.h"

class FifteenPuzzle : public Plasma::PopupApplet
{
  Q_OBJECT
  public:
    FifteenPuzzle(QObject *parent, const QVariantList &args);

    void init();
    virtual QList<QAction*> contextualActions();
    virtual QGraphicsWidget* graphicsWidget();
  protected slots:
    void configAccepted();
    
  public slots:
    void configChanged();

  private:
    void updateTimerLabel();
    void createMenu();
    QGraphicsWidget *m_graphicsWidget;
    QPixmap *m_pixmap;
    Fifteen *m_board;
    QList<QAction *> m_actions;
    QTimer m_timer;
    int m_seconds;
    Plasma::Label *m_timeLabel;
    Plasma::PushButton *m_shuffleButton;
    bool m_usePlainPieces;
    QString m_imagePath;
    bool m_showNumerals;
    Ui::fifteenPuzzleConfig ui;

  private slots:
    void createConfigurationInterface(KConfigDialog *parent);
    void startTimer();
    void updateTimer();
    void cancelTimer();
};

K_EXPORT_PLASMA_APPLET(fifteenPuzzle, FifteenPuzzle)

#endif
