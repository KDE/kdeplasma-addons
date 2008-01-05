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

#include "fileWatcherConfig.h"

#include <QFontDialog>
#include <QColorDialog>

FileWatcherConfig::FileWatcherConfig(QWidget *parent)
                  :KDialog(parent)
{ 
  setButtons( KDialog::Close );
  setCaption(i18n("File Watcher Settings"));
  
  ui.setupUi(mainWidget());
  mainWidget()->layout()->setMargin(0);
  urlRequester = new KUrlRequester(this);
  ui.hboxLayout->addWidget(urlRequester);
  urlRequester->setFocus();
 
  QObject::connect(urlRequester,SIGNAL(returnPressed(const QString&)),this,SLOT(returnPressed(const QString&)));
  QObject::connect(urlRequester,SIGNAL(urlSelected(const KUrl&)),this,SLOT(pathSelected(const KUrl&)));
  QObject::connect(ui.pb_font,SIGNAL(clicked()),this,SLOT(fontPressed()));
  QObject::connect(ui.pb_fontColor,SIGNAL(clicked()),this,SLOT(fontColorPressed()));
  QObject::connect(ui.sb_maxRows,SIGNAL(valueChanged(int)),this,SLOT(maxRowsValueChanged(int)));
}

void FileWatcherConfig::maxRowsValueChanged(int rows)
{
  emit maxRowsChanged(rows); 
}

void FileWatcherConfig::fontPressed()
{
  font = QFontDialog::getFont(NULL,font); 
  emit fontChanged(font);
}

void FileWatcherConfig::fontColorPressed()
{
  fontColor = QColorDialog::getColor(fontColor);
  emit fontColorChanged(fontColor);
}

void FileWatcherConfig::pathSelected(const KUrl &url)
{
  QFileInfo file(url.toLocalFile());
  if (file.isFile())
    emit newFile(file.absoluteFilePath());
}

void FileWatcherConfig::returnPressed(const QString& path)
{
  QFileInfo file(path);
  if (file.isFile())
    emit newFile(path);
}

#include "fileWatcherConfig.moc"

