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

#include <KComboBox>
#include <KLocale>
#include <KStandardDirs>

#include "configdialog.h"
#include "picture.h"

ConfigDialog::ConfigDialog(QWidget *parent)
        : QWidget(parent)
{
    ui.setupUi(this);

    ui.addDirButton->setIcon(KIcon("list-add"));
    ui.removeDirButton->setIcon(KIcon("list-remove"));
    ui.slideShowDelay->setMinimumTime(QTime(0, 0, 1)); // minimum to 1 second

    QString monitorPath = KStandardDirs::locate("data",  "kcontrol/pics/monitor.png");
    // Size of monitor image: 200x186
    // Geometry of "display" part of monitor image: (23,14)-[151x115]
    ui.monitorLabel->setPixmap(QPixmap(monitorPath));
    ui.monitorLabel->setWhatsThis(i18n(
                                      "This picture of a monitor contains a preview of "
                                      "the picture you currently have in your frame."));
    m_preview = new QLabel(ui.monitorLabel);
    m_preview->setScaledContents(true);
    m_preview->setGeometry(23, 14, 151, 115);
    m_preview->show();

    connect(ui.picRequester, SIGNAL(urlSelected(const KUrl &)), this, SLOT(changePreview(const KUrl &)));
    connect(ui.picRequester->comboBox(), SIGNAL(activated(const QString &)), this, SLOT(changePreview(const QString &)));
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

void ConfigDialog::setRandom(bool random)
{
    ui.randomCheckBox->setChecked(random);
}

bool ConfigDialog::random() const
{
    return ui.randomCheckBox->isChecked();
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

void ConfigDialog::setFrameColor(const QColor& frameColor)
{
    ui.changeFrameColor->setColor(frameColor);
}

QColor ConfigDialog::frameColor() const
{
    return ui.changeFrameColor->color();
}

void ConfigDialog::setCurrentUrl(const KUrl& currentUrl)
{
    ui.picRequester->setUrl(currentUrl);
}

KUrl ConfigDialog::currentUrl() const
{
    return ui.picRequester->url();
}

void ConfigDialog::previewPicture(const QImage &image)
{
    QImage scaledImage = image.scaled(QRect(23, 14, 151, 115).size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_preview->setPixmap(QPixmap::fromImage(scaledImage));
}

void ConfigDialog::changePreview(const KUrl &path)
{
    Picture myPicture;
    previewPicture(myPicture.setPicture(path));
}

void ConfigDialog::changePreview(const QString &path)
{
    Picture myPicture;
    previewPicture(myPicture.setPicture(KUrl(path)));
}

void ConfigDialog::setSmoothScaling(bool smooth)
{
    ui.smoothScaling->setChecked(smooth);
}

bool ConfigDialog::smoothScaling() const
{
    return ui.smoothScaling->isChecked();
}

