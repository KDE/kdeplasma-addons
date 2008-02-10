/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
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

#include <klocale.h>

#include "configdialog.h"


ConfigDialog::ConfigDialog( QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18nc( "@title:window", "Configure Frame" ) );

    setButtons( Ok | Cancel | Apply );
    setDefaultButton( Ok );
    showButtonSeparator( true );

    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    setMainWidget(widget);
    
    ui.addDirButton->setIcon(KIcon("list-add"));
    ui.removeDirButton->setIcon(KIcon("list-remove"));
    ui.slideShowDelay->setMinimumTime(QTime(0, 0, 1)); // minimum to 1 second
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setRoundCorners(bool round)
{
    ui.roundCheckBox->setChecked(round);
}

bool ConfigDialog::roundCorners() const
{
    return ui.roundCheckBox->isChecked();
}

void ConfigDialog::setShadow(bool shadow)
{
    ui.shadowCheckBox->setChecked(shadow);
}

bool ConfigDialog::shadow() const
{
    return ui.shadowCheckBox->isChecked();
}

void ConfigDialog::setShowFrame(bool show)
{
    ui.frameCheckBox->setChecked(show);
}

bool ConfigDialog::showFrame() const
{
    return ui.frameCheckBox->isChecked();
}

void ConfigDialog::setFrameColor(QColor frameColor)
{
    ui.changeFrameColor->setColor(frameColor);
}

QColor ConfigDialog::frameColor() const
{
    return ui.changeFrameColor->color();
}

void ConfigDialog::setCurrentUrl(KUrl currentUrl)
{
    ui.picRequester->setUrl(currentUrl);
}

KUrl ConfigDialog::currentUrl() const
{
    return ui.picRequester->url();
}