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

//Plasma
#include <Plasma/Theme>

static const char defaultImage[] = "fifteenPuzzle/greensquare";

FifteenPuzzle::FifteenPuzzle(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_configDialog(0)
{
  setHasConfigurationInterface(true);
  m_board = new Fifteen(this);
  QGraphicsLinearLayout * lay = new QGraphicsLinearLayout(this);
  lay->addItem(m_board);
  lay->setContentsMargins(0,0,0,0);
  setLayout(lay);
  m_board->resize(192, 192); // 48 * 4 = 192
  resize(m_board->geometry().size());
}

void FifteenPuzzle::init()
{
  createMenu();

  KConfigGroup cg = config();

  m_imagePath = cg.readEntry("ImagePath", QString());
  m_usePlainPieces = m_imagePath.isEmpty() || cg.readEntry("UsePlainPieces", true);
  m_showNumerals = cg.readEntry("ShowNumerals", true);

  // make sure nobody messed up with the config file
  if (!m_usePlainPieces) {
      if (!QFile::exists(m_imagePath)) {
          // lets see if it exists in the theme
          m_imagePath = Plasma::Theme::defaultTheme()->imagePath(m_imagePath);
      }

      if (m_imagePath.isEmpty()) {
          m_usePlainPieces = true;
      }
  }

  updateBoard();
}

void FifteenPuzzle::constraintsEvent(Plasma::Constraints constraints)
{
  if (constraints & Plasma::SizeConstraint) {
    m_board->resize(contentsRect().size());
  }
}

void FifteenPuzzle::createConfigurationInterface(KConfigDialog *parent)
{
  m_configDialog = new FifteenPuzzleConfig();
  connect(m_configDialog, SIGNAL(shuffle()), m_board, SLOT(shuffle()));
  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
  parent->addPage(m_configDialog, i18n("General"), icon());

  if (m_usePlainPieces) {
    m_configDialog->ui.rb_identical->setChecked(true);
  } else {
    m_configDialog->ui.rb_split->setChecked(true);
  }
  m_configDialog->ui.urlRequester->setUrl(m_imagePath);
  m_configDialog->ui.cb_showNumerals->setChecked(m_showNumerals);
  m_configDialog->show();
}

void FifteenPuzzle::configAccepted()
{
  KConfigGroup cg = config();

  m_imagePath = m_configDialog->ui.urlRequester->url().path();
  m_usePlainPieces = m_imagePath.isEmpty() || m_configDialog->ui.rb_identical->isChecked();
  m_showNumerals = m_configDialog->ui.cb_showNumerals->isChecked();

  cg.writeEntry("ShowNumerals", m_showNumerals);
  cg.writeEntry("UsePlainPieces", m_usePlainPieces);
  cg.writeEntry("ImagePath", m_imagePath);

  updateBoard();

  emit configNeedsSaving();
}

void FifteenPuzzle::updateBoard()
{
  m_board->setShowNumerals(m_showNumerals);
  m_board->setImage(m_usePlainPieces ? QLatin1String(defaultImage) : m_imagePath, m_usePlainPieces);
}

void FifteenPuzzle::createMenu()
{
  QAction *shuffle = new QAction(i18n("Shuffle Pieces"), this);
  m_actions.append(shuffle);
  connect(shuffle, SIGNAL(triggered(bool)), m_board, SLOT(shuffle()));
}

QList<QAction*> FifteenPuzzle::contextualActions()
{
    return m_actions;
}


#include "fifteenPuzzle.moc"

