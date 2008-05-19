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

#ifndef FILEWATCHERCONFIG_H
#define FILEWATCHERCONFIG_H

#include <KUrlRequester>

#include "ui_fileWatcherConfig.h"

class FileWatcherConfig : public QWidget 
{
  Q_OBJECT

  public:
    FileWatcherConfig(QWidget *parent=0);

  private slots:
    void returnPressed(const QString& path);
    void pathSelected(const KUrl& url);
    void fontPressed();
    void fontColorPressed();
    void maxRowsValueChanged(int rows);
 
  private:
    Ui::fileWatcherConfig ui;
    KUrlRequester *urlRequester;
    QFont font;
    QColor fontColor;
  
  signals:
    void fontChanged(QFont);
    void fontColorChanged(QColor);
    void newFile(const QString& path);
    void maxRowsChanged(int rows); 
};

#endif

