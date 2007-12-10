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

#include <QMessageBox>

FifteenPuzzleConfig::FifteenPuzzleConfig(QWidget *parent)
                  :KDialog(parent)
{ 
  setButtons( KDialog::Close );
  setCaption(i18n("Configure Fifteen Puzzle"));
  
  ui.setupUi(mainWidget());
  ui.gb_imageOptions->hide();
  urlRequester = new KUrlRequester(this);
  ui.hboxLayout->addWidget(urlRequester);

  ui.rb_identical->toggle();
  ui.cb_showNumerals->setCheckState(Qt::Checked);

  QObject::connect(ui.rb_identical,SIGNAL(toggled(bool)),this,SLOT(identicalToggled(bool)));
  QObject::connect(ui.rb_split,SIGNAL(toggled(bool)),this,SLOT(splitPixmapToggled(bool)));
  
  QObject::connect(urlRequester,SIGNAL(returnPressed(const QString&)),this,SLOT(returnPressed(const QString&)));
  QObject::connect(urlRequester,SIGNAL(urlSelected(const KUrl&)),this,SLOT(pathSelected(const KUrl&)));


  QObject::connect(ui.pb_shuffle,SIGNAL(clicked()),this,SIGNAL(shuffle()));
  QObject::connect(ui.cb_showNumerals,SIGNAL(stateChanged(int)),this,SLOT(showNumeralsChanged(int)));

}

void FifteenPuzzleConfig::showNumeralsChanged(int status)
{
  if (status == Qt::Unchecked)
    emit setNumerals(false);
  else
    emit setNumerals(true);
}

void FifteenPuzzleConfig::identicalToggled(bool toggled)
{
  if (toggled)
  {
    emit setIdentical();
    ui.cb_showNumerals->setCheckState(Qt::Checked);
  }
}


void FifteenPuzzleConfig::splitPixmapToggled(bool toggled)
{
  if (toggled){
    if (urlRequester->url().hasPath())
      pathSelected(urlRequester->url());  
    
    ui.gb_imageOptions->show();
    urlRequester->setFocus();
  }
  else
    ui.gb_imageOptions->hide();
}

void FifteenPuzzleConfig::pathSelected(const KUrl &url)
{
  QFileInfo file(url.toLocalFile());
  if (isValidImage(file.absoluteFilePath()) && file.isFile() ) 
    emit setSplitPixmap(file.absoluteFilePath());
}

void FifteenPuzzleConfig::returnPressed(const QString& path)
{
  QFileInfo file(path);
  if (isValidImage(path) && file.isFile())
    emit setSplitPixmap(path);
}


bool FifteenPuzzleConfig::isValidImage(const QString &path)
{
  QPixmap pixmap(path);
  if (pixmap.isNull())
  {
    QMessageBox::critical(NULL, i18n("Failed to open"), i18n("Could not open selected image. Please choose another."));
    return false;
  }

  return true;
}

#include "fifteenPuzzleConfig.moc"

