/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
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
#include "ui_advancedsettings.h"
#include "ui_comicSettings.h"

#include <QtGui/QWidget>
#include <QTime>

#include <KNS3/Entry>
#include <Plasma/DataEngine>

class ComicModel;
class KConfigDialog;
class QCheckBox;
class QComboBox;
class QSortFilterProxyModel;

namespace KNS3 {
    class DownloadDialog;
    class DownloadManager;
}

namespace Plasma {
    class DataEngine;
}

class ConfigWidget;

class ComicUpdater : public QObject
{
    Q_OBJECT

    public:
        explicit ComicUpdater( QObject *parent = 0 );
        ~ComicUpdater();

        void init( const KConfigGroup &group );

        void load();
        void save();
        void applyConfig( ConfigWidget *widget );

    private slots:
         /**
          * Will check if an update is needed, if so will search
          * for updates and do them automatically
          */
        void checkForUpdate();
        void slotUpdatesFound( const KNS3::Entry::List &entries );

    private:
        KNS3::DownloadManager *downloadManager();

    private:
        KNS3::DownloadManager *mDownloadManager;
        KConfigGroup mGroup;
        int mUpdateIntervall;
        QDateTime mLastUpdate;
        QTimer *m_updateTimer;

};

class ConfigWidget : public QWidget
{
        Q_OBJECT
    public:
        ConfigWidget( Plasma::DataEngine *engine, ComicModel *model, QSortFilterProxyModel *proxy, KConfigDialog *parent );
        ~ConfigWidget();

        void setShowComicUrl( bool show );
        bool showComicUrl() const;
        void setShowComicAuthor( bool show );
        bool showComicAuthor() const;
        void setShowComicTitle( bool show );
        bool showComicTitle() const;
        void setShowComicIdentifier( bool show );
        bool showComicIdentifier() const;
        void setShowErrorPicture( bool show );
        bool showErrorPicture() const;
        void setArrowsOnHover( bool arrows );
        bool arrowsOnHover() const;
        void setMiddleClick( bool checked );
        bool middleClick() const;
        void setTabView( int tabView );
        int tabView() const;
        int maxComicLimit() const;
        void setMaxComicLimit( int limit );
        void setUpdateIntervall( int days );
        int updateIntervall() const;
        void setCheckNewComicStripsIntervall( int minutes );
        int checkNewComicStripsIntervall() const;

        QWidget *comicSettings;
        QWidget *appearanceSettings;
        QWidget *advancedSettings;

    Q_SIGNALS:
        void maxSizeClicked();
        void enableApply();

    public slots:
         void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );

    protected slots:
        void getNewStuff();

    private:
        Ui::ComicSettings comicUi;
        Ui::AppearanceSettings appearanceUi;
        Ui::AdvancedSettings advancedUi;
        Plasma::DataEngine *mEngine;
        ComicModel *mModel;
        QSortFilterProxyModel *mProxyModel;
        KNS3::DownloadDialog* mNewStuffDialog;
};

#endif
