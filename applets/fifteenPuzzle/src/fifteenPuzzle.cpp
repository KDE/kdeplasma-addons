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

#include "fifteenPuzzle.h"

FifteenPuzzle::FifteenPuzzle(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), config_dialog(0)
{
  setHasConfigurationInterface(true);
  board = new Fifteen(this);
  board->setRect(0, 0, 192, 192); // 48 * 4 = 192
}

QSizeF FifteenPuzzle::contentSizeHint() const
{ 
  return board->boundingRect().size();
}

void FifteenPuzzle::constraintsUpdated(Plasma::Constraints constraints)
{
  if (constraints & Plasma::SizeConstraint)
  {
    QSizeF size = this->contentSize();
    board->resetTransform();
    board->scale(size.width()/192,size.height()/192);
  }
}

void FifteenPuzzle::showConfigurationInterface()
{
  if (config_dialog == 0)
  {
    config_dialog = new FifteenPuzzleConfig();
    QObject::connect(config_dialog,SIGNAL(setSplitPixmap(QString)),board, SLOT(setSplitPixmap(QString)));
    QObject::connect(config_dialog,SIGNAL(setIdentical()),board, SLOT(setIdentical()));
    QObject::connect(config_dialog,SIGNAL(setNumerals(bool)),board, SLOT(setNumerals(bool)));
    QObject::connect(config_dialog,SIGNAL(shuffle()),board, SLOT(shuffle()));
  }
  config_dialog->show();
}

#include "fifteenPuzzle.moc"
