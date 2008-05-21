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

#include "fifteenPuzzleConfig.h"

#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtGui/QPixmap>

FifteenPuzzleConfig::FifteenPuzzleConfig(QWidget *parent)
    : KDialog(parent)
{
  setCaption(i18nc("@title:window", "Configure Fifteen Puzzle"));
  
  setButtons(KDialog::None);

  ui.setupUi(mainWidget());

  connect(ui.pb_shuffle, SIGNAL(clicked()), this, SIGNAL(shuffle()));
}

void FifteenPuzzleConfig::slotButtonClicked(int button)
{
  if (ui.rb_identical->isChecked() == false) {
    switch (button) {
      case KDialog::Ok:
      case KDialog::Apply: {
        QString path = ui.urlRequester->url().path();
        if (!QFile::exists(path) ||
            QPixmap(path).isNull()) {
          KMessageBox::sorry(this,
            i18nc("@body:window", "You have to provide a valid image"),
            i18nc("@title:window", "Configure Fifteen Puzzle"),
            KMessageBox::Notify);
        } else {
          KDialog::slotButtonClicked(button);
        }
        return;
      }
      default:
        break;
    }
  }
  KDialog::slotButtonClicked(button);
}

#include "fifteenPuzzleConfig.moc"

