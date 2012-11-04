/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
 *   Copyright (C) 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>      *
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

#ifndef COMIC_H
#define COMIC_H

#include "comicdata.h"

#include <QtCore/QDate>

#include <KUrl>
#include <Plasma/DataEngine>
#include <Plasma/PopupApplet>

#include "activecomicmodel.h"

namespace Plasma {
    class DeclarativeWidget;
}

class CheckNewStrips;
class ComicModel;
class ConfigWidget;
class KAction;
class KJob;
class QAction;
class QGraphicsLayout;
class QSortFilterProxyModel;
class QTimer;
class SavingDir;

class ComicApplet : public Plasma::PopupApplet
{
    Q_OBJECT
    Q_PROPERTY(QObject * comicsModel READ comicsModel NOTIFY comicModelChanged)
    Q_PROPERTY(bool showComicUrl READ showComicUrl WRITE setShowComicUrl NOTIFY showComicUrlChanged)
    Q_PROPERTY(bool showComicAuthor READ showComicAuthor WRITE setShowComicAuthor NOTIFY showComicAuthorChanged)
    Q_PROPERTY(bool showComicTitle READ showComicTitle WRITE setShowComicTitle NOTIFY showComicTitleChanged)
    Q_PROPERTY(bool showComicIdentifier READ showComicIdentifier WRITE setShowComicIdentifier NOTIFY showComicIdentifierChanged)
    Q_PROPERTY(bool showErrorPicture READ showErrorPicture WRITE setShowErrorPicture NOTIFY showErrorPictureChanged)
    Q_PROPERTY(bool arrowsOnHover READ arrowsOnHover WRITE setArrowsOnHover NOTIFY arrowsOnHoverChanged)
    Q_PROPERTY(bool middleClick READ middleClick WRITE setMiddleClick NOTIFY middleClickChanged)
    Q_PROPERTY(QVariantHash comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet();

        void init();
        virtual QList<QAction*> contextualActions();

        QGraphicsWidget *graphicsWidget();

        //For QML
        QObject *comicsModel();
        QVariantHash comicData();

        bool showComicUrl() const;
        void setShowComicUrl(bool show);

        bool showComicAuthor() const;
        void setShowComicAuthor(bool show);

        bool showComicTitle() const;
        void setShowComicTitle(bool show);

        bool showComicIdentifier() const;
        void setShowComicIdentifier(bool show);

        bool showErrorPicture() const;
        void setShowErrorPicture(bool show);

        bool arrowsOnHover() const;
        void setArrowsOnHover(bool show);

        bool middleClick() const;
        void setMiddleClick(bool show);

        bool showActualSize() const;
        void setShowActualSize(bool show);

        Q_INVOKABLE bool checkAuthorization(const QString &permissionName) { return hasAuthorization(permissionName); }
        //End for QML
Q_SIGNALS:
    void comicModelChanged();
    void showComicUrlChanged();
    void showComicAuthorChanged();
    void showComicTitleChanged();
    void showComicIdentifierChanged();
    void showErrorPictureChanged();
    void arrowsOnHoverChanged();
    void middleClickChanged();
    void comicDataChanged();
    void tabHighlightRequest(const QString &id, bool highlight);
    void showNextNewStrip();
    void showActualSizeChanged();

    public Q_SLOTS:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
        void createConfigurationInterface( KConfigDialog *parent );

    private Q_SLOTS:
        void slotTabChanged( const QString &newIdentifier );
        void slotNextDay();
        void slotPreviousDay();
        void slotFirstDay();
        void slotCurrentDay();
        void slotFoundLastStrip( int index, const QString &identifier, const QString &suffix );
        void slotGoJump();
        void slotSaveComicAs();
        void slotScaleToContent();
        void slotShop();
        void slotStorePosition();
        void applyConfig();
        void checkDayChanged();
        void createComicBook();
        void slotArchive( int archiveType, const KUrl &dest, const QString &fromIdentifier, const QString &toIdentifier );
        void slotArchiveFinished( KJob *job );

    public slots:
        void configChanged();
        Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());
        Q_INVOKABLE void goJump() { slotGoJump();}
        Q_INVOKABLE void shop() { slotShop();}
        Q_INVOKABLE void tabChanged(const QString &newIdentifier) { slotTabChanged(newIdentifier);}

    protected:
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    private:
        void changeComic( bool differentComic );
        void updateUsedComics();
        void updateContextMenu();
        void updateView();
        void saveConfig();
        bool isInPanel() const;
        void refreshComicData();
        void setTabHighlighted(const QString &id, bool highlight);
        bool hasHighlightedTabs();
        bool isTabHighlighted(int index) const;

    private:
        static const int CACHE_LIMIT;
        ComicModel *mModel;
        QSortFilterProxyModel *mProxy;
        ActiveComicModel mActiveComicModel;
        QVariantHash mComicData;

        QDate mCurrentDay;

        QString mIdentifierError;
        QString mOldSource;
        ConfigWidget *mConfigWidget;
        bool mDifferentComic;
        bool mShowComicUrl;
        bool mShowComicAuthor;
        bool mShowComicTitle;
        bool mShowComicIdentifier;
        bool mShowErrorPicture;
        bool mArrowsOnHover;
        bool mMiddleClick;
        int mCheckNewComicStripsIntervall;
        CheckNewStrips *mCheckNewStrips;
        QTimer *mDateChangedTimer;
        QList<QAction*> mActions;
        Plasma::DeclarativeWidget *mDeclarativeWidget;
        QAction *mActionGoFirst;
        QAction *mActionGoLast;
        QAction *mActionGoJump;
        QAction *mActionScaleContent;
        QAction *mActionShop;
        QAction *mActionStorePosition;
        KAction *mActionNextNewStripTab;
        QSizeF mMaxSize;
        QSizeF mLastSize;
        QSizeF mIdealSize;
        Plasma::DataEngine *mEngine;

        //Tabs
        bool mTabAdded;
        QStringList mTabIdentifier;

        ComicData mCurrent;
        SavingDir *mSavingDir;
};

K_EXPORT_PLASMA_APPLET( comic, ComicApplet )

#endif
