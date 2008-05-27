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

#ifndef FIFTEENPUZZLECONFIG_H
#define FIFTEENPUZZLECONFIG_H

#include <KDialog>

#include "ui_fifteenPuzzleConfig.h"

class FifteenPuzzleConfig : public KDialog
{
  Q_OBJECT
  public:
    FifteenPuzzleConfig(QWidget *parent = 0);

    Ui::fifteenPuzzleConfig ui;

  protected Q_SLOTS:
    void slotButtonClicked(int button);

  signals:
    void shuffle();
};

#endif
