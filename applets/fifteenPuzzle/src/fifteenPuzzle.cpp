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
#include <QGraphicsAnchorLayout>

//KDE
#include <KConfigDialog>
#include <KIcon>

//Plasma
#include <Plasma/Theme>

static const char defaultImage[] = "fifteenPuzzle/blanksquare";

FifteenPuzzle::FifteenPuzzle(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_pixmap(0),
      m_seconds(0)
{
  setHasConfigurationInterface(true);
  setPopupIcon("fifteenpuzzle");

  m_timer.setInterval(1000);
  m_timer.setSingleShot(false);
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

  m_graphicsWidget = new QGraphicsWidget(this);
  QGraphicsAnchorLayout *layout = new QGraphicsAnchorLayout();
  m_graphicsWidget->setLayout(layout);

  m_board = new Fifteen(m_graphicsWidget);
  connect(m_board, SIGNAL(started()), this, SLOT(startTimer()));
  connect(m_board, SIGNAL(solved()), &m_timer, SLOT(stop()));
  connect(m_board, SIGNAL(aborted()), this, SLOT(cancelTimer()));
  layout->addAnchors(m_board, layout, Qt::Horizontal);
  layout->addAnchor(m_board, Qt::AnchorTop, layout, Qt::AnchorTop);

  m_shuffleButton = new Plasma::PushButton(m_graphicsWidget);
  m_shuffleButton->setText(i18n("Shuffle"));
  m_shuffleButton->setIcon(KIcon("roll"));
  m_shuffleButton->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed,
                                             QSizePolicy::PushButton));
  connect(m_shuffleButton, SIGNAL(clicked()), m_board, SLOT(shuffle()));
  layout->addAnchor(m_shuffleButton, Qt::AnchorTop, m_board, Qt::AnchorBottom);
  layout->addCornerAnchors(m_shuffleButton, Qt::BottomLeftCorner, layout, Qt::BottomLeftCorner);

  m_timeLabel = new Plasma::Label(m_graphicsWidget);
  m_timeLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed,
                                         QSizePolicy::Label));
  updateTimerLabel();
  layout->addAnchor(m_timeLabel, Qt::AnchorTop, m_board, Qt::AnchorBottom);
  layout->addCornerAnchors(m_timeLabel, Qt::BottomRightCorner, layout, Qt::BottomRightCorner);

  layout->addAnchor(m_shuffleButton, Qt::AnchorRight, m_timeLabel, Qt::AnchorLeft)->setSizePolicy(QSizePolicy::MinimumExpanding);
}

void FifteenPuzzle::init()
{
  createMenu();

  configChanged();
}

QGraphicsWidget* FifteenPuzzle::graphicsWidget()
{
    return m_graphicsWidget;
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
    
    connect(ui.size, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.color, SIGNAL(activated(QColor)), parent, SLOT(settingsModified()));
    connect(ui.rb_identical, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.rb_split, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.cb_showNumerals, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
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

void FifteenPuzzle::startTimer()
{
    m_seconds = 0;
    updateTimerLabel();
    m_timer.start();
}

void FifteenPuzzle::updateTimer()
{
    m_seconds++;
    updateTimerLabel();
}

void FifteenPuzzle::cancelTimer()
{
    m_timer.stop();
    m_seconds = 0;
    updateTimerLabel();
}

void FifteenPuzzle::updateTimerLabel()
{
    QString min = QString::number(m_seconds / 60).rightJustified(2, QLatin1Char('0'), false);
    QString sec = QString::number(m_seconds % 60).rightJustified(2, QLatin1Char('0'), false);
    m_timeLabel->setText(i18nc("The time since the puzzle started, in minutes and seconds",
                               "Time: %1:%2", min, sec));
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

