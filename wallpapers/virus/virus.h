/*
  Copyright (c) 2009 by Beat Wolf <asraniel@fryx.ch>
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef VIRUS_HEADER
#define VIRUS_HEADER

#include <QTimer>
#include <QPixmap>
#include <QStringList>

#include <Plasma/Wallpaper>
#include <Plasma/Package>

#include "ui_virusconfig.h"

#include "alife.h"

class KFileDialog;
class BackgroundListModel;

class Virus : public Plasma::Wallpaper
{
    Q_OBJECT
    public:
        Virus(QObject* parent, const QVariantList& args);
        ~Virus();

        virtual void save(KConfigGroup &config);
        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        virtual QWidget* createConfigurationInterface(QWidget* parent);
        void updateScreenshot(QPersistentModelIndex index);
    signals:
        void settingsChanged(bool);
    protected slots:
        void positioningChanged(int index);
        void getNewWallpaper();
        void colorChanged(const QColor& color);
        void pictureChanged(QModelIndex index);
        void browse();
        void updateBackground(const QImage &img);
        void showFileDialog();
        void configWidgetDestroyed();

        void requestUpdate();
        void intervalChanged(int interval);
        void maxCellsChanged(int interval);
        void showCellsChanged(int state);
        void virusUpdated();
        void modified();

    protected:
        void init(const KConfigGroup &config);
        void fillMetaInfo(Plasma::Package *b);
        bool setMetadata(QLabel *label, const QString &text);
        void renderWallpaper(const QString& image = QString());
        void suspendStartup(bool suspend); // for ksmserver
        void calculateGeometry();
        void setSingleImage();
        QString cacheId() const;

    private:
        Plasma::Wallpaper::ResizeMethod m_resizeMethod;
        QStringList m_dirs;
        QString m_wallpaper;
        QColor m_color;
        QStringList m_usersWallpapers;

        QWidget* m_configWidget;
        Ui::ImageConfig m_uiVirus;
        QString m_mode;
        QPixmap m_pixmap;
        QPixmap m_oldPixmap;
        QPixmap m_oldFadedPixmap;
        int m_currentSlide;
        BackgroundListModel *m_model;
        KFileDialog *m_dialog;
        QSize m_size;
        QString m_img;
        QDateTime m_previousModified;
        bool m_randomize;
        bool m_startupResumed;

        QTimer m_timer;
        
        Alife alife;
};

#endif
