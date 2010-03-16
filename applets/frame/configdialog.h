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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "ui_imageSettings.h"
#include "ui_appearanceSettings.h"

#include "picture.h"

class ConfigDialog : public QObject
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent);
    ~ConfigDialog();
    // Appearance
    /// Round corners for the frame
    void setRoundCorners(bool round);
    bool roundCorners() const;
    /// Randomness for the slideshow
    void setRandom(bool random);
    bool random() const;
    /// Set a shadow for the frame
    void setShadow(bool round);
    bool shadow() const;
    /// Show the frame or not
    void setShowFrame(bool show);
    bool showFrame() const;
    /// Frame Color
    void setFrameColor(const QColor& frameColor);
    QColor frameColor() const;
    // Static Picture
    void setCurrentUrl(const KUrl& currentUrl);
    KUrl currentUrl() const;
    /// Designer Config file
    Ui::ImageSettings imageUi;
    Ui::AppearanceSettings appearanceUi;
    QWidget *imageSettings;
    QWidget *appearanceSettings;
    /// Image Preview
    /// Allow to preview each new chosen picture
    void previewPicture(const QImage &img);

private slots:
    /// Update preview when URL changes via the file dialog
    void changePreview(const KUrl &);
    /// Update preview when URL changes via the combobox
    void changePreview(const QString &);
    /// The image is loaded, update the preview
    void pictureLoaded(QImage image);
    /// The preview is scaled
    void previewScaled(const QImage &);

private:
    Picture *m_picture;
    QLabel *m_preview;
};

#endif
