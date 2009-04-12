/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include "ui_appearanceSettings.h"
#include "ui_comicSettings.h"

#include <QtGui/QWidget>
#include <QTime>

#include <Plasma/DataEngine>

class ComicModel;
class QCheckBox;
class QComboBox;
class QSortFilterProxyModel;
class QPushButton;

class ConfigWidget : public QWidget
{
        Q_OBJECT
    public:
        ConfigWidget( Plasma::DataEngine *engine, QWidget *parent );
        ~ConfigWidget();

        void setComicIdentifier( const QStringList &comics );
        QStringList comicIdentifier() const;

        void setShowComicUrl( bool show );
        bool showComicUrl() const;
        void setShowComicAuthor( bool show );
        bool showComicAuthor() const;
        void setShowComicTitle( bool show );
        bool showComicTitle() const;
        void setShowComicIdentifier( bool show );
        bool showComicIdentifier() const;
        void setArrowsOnHover( bool arrows );
        bool arrowsOnHover() const;
        void setMiddleClick( bool checked );
        bool middleClick() const;
        void setTabSwitchTime( const QTime &time );
        QTime tabSwitchTime() const;
        void setHideTabBar( bool hide );
        bool hideTabBar() const;
        void setUseTabs( bool use );
        bool useTabs() const;
        void setSwitchTabs( bool switchTabs );
        bool switchTabs() const;
        QStringList comicName() const;

        QWidget *comicSettings;
        QWidget *appearanceSettings;

    Q_SIGNALS:
        void maxSizeClicked();


    protected slots:
        void getNewStuff();

    private:
        Ui::ComicSettings comicUi;
        Ui::AppearanceSettings appearanceUi;
        ComicModel *mModel;
        Plasma::DataEngine *mEngine;
        QSortFilterProxyModel *mProxyModel;
};

#endif
