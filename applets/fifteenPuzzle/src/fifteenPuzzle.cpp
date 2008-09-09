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

//Qt
#include <QtCore/QFile>
#include <QGraphicsLinearLayout>

//KDE
#include <KConfigDialog>


FifteenPuzzle::FifteenPuzzle(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), configDialog(0)
{
  setHasConfigurationInterface(true);
  board = new Fifteen(this);
  QGraphicsLinearLayout * lay = new QGraphicsLinearLayout(this);
  lay->addItem(board);
  lay->setContentsMargins(0,0,0,0);
  setLayout(lay);
  board->resize(192, 192); // 48 * 4 = 192
  resize(board->geometry().size());
}

void FifteenPuzzle::init()
{
  createMenu();

  KConfigGroup cg = config();

  usePlainPieces = cg.readEntry("UsePlainPieces", true);
  imagePath = cg.readEntry("ImagePath", QString());
  showNumerals = cg.readEntry("ShowNumerals", true);

  // make sure nobody messed up with the config file
  if (!usePlainPieces &&
      (!QFile::exists(imagePath) ||
       QPixmap(imagePath).isNull())) {
    usePlainPieces = true;
    imagePath = QString();
  }

  updateBoard();
}

void FifteenPuzzle::constraintsEvent(Plasma::Constraints constraints)
{
  if (constraints & Plasma::SizeConstraint) {
    QSizeF size = contentsRect().size();
    board->resetTransform();
    board->scale(size.width() / 192, size.height() / 192);
  }
}

void FifteenPuzzle::createConfigurationInterface(KConfigDialog *parent)
{
  configDialog = new FifteenPuzzleConfig();
  connect(configDialog, SIGNAL(shuffle()), board, SLOT(shuffle()));
  parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
  parent->addPage(configDialog, parent->windowTitle(), icon());

  if (usePlainPieces) {
    configDialog->ui.rb_identical->setChecked(true);
  }
  else {
    configDialog->ui.rb_split->setChecked(true);
  }
  configDialog->ui.urlRequester->setUrl(imagePath);
  configDialog->ui.cb_showNumerals->setChecked(showNumerals);
  configDialog->show();
}

void FifteenPuzzle::configAccepted()
{
  KConfigGroup cg = config();

  usePlainPieces = configDialog->ui.rb_identical->isChecked();
  imagePath = configDialog->ui.urlRequester->url().path();
  showNumerals = configDialog->ui.cb_showNumerals->isChecked();

  if ( !imagePath.isEmpty() )
  {
      cg.writeEntry("UsePlainPieces", usePlainPieces);
      cg.writeEntry("ImagePath", imagePath);
  }
  else
  {
      usePlainPieces = true;
      cg.writeEntry("UsePlainPieces", true);
  }
  cg.writeEntry("ShowNumerals", showNumerals);
  updateBoard();

  emit configNeedsSaving();
}

void FifteenPuzzle::updateBoard()
{
  if (usePlainPieces) {
    board->setIdentical();
  }
  else {
    board->setSplitPixmap(imagePath);
    board->setNumerals(showNumerals);
  }
}

void FifteenPuzzle::createMenu()
{
  QAction *shuffle = new QAction(i18n("Shuffle Pieces"), this);
  actions.append(shuffle);
  connect(shuffle, SIGNAL(triggered(bool)), board, SLOT(shuffle()));
}

QList<QAction*> FifteenPuzzle::contextualActions()
{
    return actions;
}


#include "fifteenPuzzle.moc"

