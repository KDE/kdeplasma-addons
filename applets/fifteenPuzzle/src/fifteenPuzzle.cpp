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
#include <KIcon>

//Plasma
#include <Plasma/Theme>

static const char defaultImage[] = "fifteenPuzzle/blanksquare";

FifteenPuzzle::FifteenPuzzle(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
{
  setHasConfigurationInterface(true);
  setPopupIcon("fifteenpuzzle");
  m_board = new Fifteen(this);
  m_pixmap = 0;
  connect(m_board, SIGNAL(puzzleSorted(int)), this, SLOT(showSolvedMessage(int)));
}

void FifteenPuzzle::init()
{
  createMenu();

  configChanged();
}

QGraphicsWidget* FifteenPuzzle::graphicsWidget()
{
    return m_board;
}

void FifteenPuzzle::configChanged()
{
  KConfigGroup cg = config();

  m_imagePath = cg.readEntry("ImagePath", QString());
  m_usePlainPieces = m_imagePath.isEmpty() || cg.readEntry("UsePlainPieces", true);
  m_showNumerals = cg.readEntry("ShowNumerals", true);

  m_board->setColor(cg.readEntry("boardColor", QColor()));
  m_board->setSize(qMax(4, cg.readEntry("boardSize", 4)));

  if (!m_usePlainPieces) {
      if (!QFile::exists(m_imagePath)) {
          // check if it exists in the theme
          m_imagePath = Plasma::Theme::defaultTheme()->imagePath(m_imagePath);
      }
      if (m_imagePath.isEmpty()) {
          m_usePlainPieces = true;
      } else {
          if (!m_pixmap) {
              m_pixmap = new QPixmap();
          }
          m_pixmap->load(m_imagePath);
          m_board->setPixmap(m_pixmap);
      }
  }
  if (m_usePlainPieces) {
      m_board->setPixmap(0);
      m_board->setSvg(QLatin1String(defaultImage), m_usePlainPieces);
      m_showNumerals = true;

      delete m_pixmap;
      m_pixmap = 0;
  }

  m_board->setShowNumerals(m_showNumerals);
}

void FifteenPuzzle::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *page = new QWidget(parent);
    ui.setupUi(page);
    parent->addPage(page, i18n("General"), icon());

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    if (m_usePlainPieces) {
        ui.rb_identical->setChecked(true);
    } else {
        ui.rb_split->setChecked(true);
    }
    ui.urlRequester->setUrl(m_imagePath);
    ui.cb_showNumerals->setChecked(m_showNumerals);
    ui.color->setColor(m_board->color());
    ui.size->setValue(m_board->size());
}

void FifteenPuzzle::configAccepted()
{
    KConfigGroup cg = config();

    cg.writeEntry("ShowNumerals",   ui.cb_showNumerals->isChecked());
    cg.writeEntry("UsePlainPieces", ui.rb_identical->isChecked());
    cg.writeEntry("ImagePath",      ui.urlRequester->url().path());
    cg.writeEntry("boardSize",      ui.size->value());
    cg.writeEntry("boardColor",     ui.color->color());

    emit configNeedsSaving();
}

void FifteenPuzzle::showSolvedMessage(int ms)
{
  QTime overallTime(0,0,0,0);
  overallTime = overallTime.addMSecs(ms);
  Plasma::Applet::showMessage(KIcon("dialog-information"), QString("Time elapsed: %1").arg(overallTime.toString("hh:mm:ss.zzz")), Plasma::ButtonOk);
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

