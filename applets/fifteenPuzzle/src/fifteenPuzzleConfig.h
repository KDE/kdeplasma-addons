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

#ifndef FIFTEENPUZZLECONFIG_H
#define FIFTEENPUZZLECONFIG_H

#include <KDialog>
#include <KUrlRequester>

#include "ui_fifteenPuzzleConfig.h"

class FifteenPuzzleConfig : public KDialog 
{
  Q_OBJECT

  public:
    FifteenPuzzleConfig(QWidget *parent=0);

  private slots:
    void returnPressed(const QString& path);
    void pathSelected(const KUrl& url);
    void splitPixmapToggled(bool toggled);
    void identicalToggled(bool toggled);
    void showNumeralsChanged(int state);
  
  private:
    bool isValidImage(const QString &path);

    Ui::fifteenPuzzleConfig ui;
    KUrlRequester *urlRequester;

  
  signals:
    void setIdentical();
    void setSplitPixmap(QString path);
    void setNumerals(bool show);
    void shuffle();
};

#endif

