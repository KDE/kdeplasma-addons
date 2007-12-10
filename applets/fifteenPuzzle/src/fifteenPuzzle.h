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

#include <plasma/applet.h>

#include "fifteen.h"
#include "fifteenPuzzleConfig.h"

class FifteenPuzzle : public Plasma::Applet
{
  Q_OBJECT
  public:
    FifteenPuzzle(QObject *parent, const QVariantList &args);
    QSizeF contentSizeHint() const;
    void constraintsUpdated(Plasma::Constraints constraints);
  
  private:
    Fifteen *board;  
    FifteenPuzzleConfig *config_dialog;

  private slots:
    void showConfigurationInterface();

};

K_EXPORT_PLASMA_APPLET(fifteenPuzzle, FifteenPuzzle)

#endif
