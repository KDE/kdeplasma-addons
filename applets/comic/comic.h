/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
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

#include <QtCore/QDate>
#include <QtCore/QStringList>
#include <QtGui/QImage>

#include <plasma/applet.h>
#include <plasma/dataengine.h>

class ConfigWidget;

class ComicApplet : public Plasma::Applet
{
    Q_OBJECT

    public:
        ComicApplet( QObject *parent, const QVariantList &args );
        ~ComicApplet();

        QSizeF contentSizeHint() const;
        void paintInterface( QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect );

    public Q_SLOTS:
        void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
        void createConfigurationInterface( KConfigDialog *parent );

    private Q_SLOTS:
        void slotNextDay();
        void slotPreviousDay();
        void applyConfig();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* );

    private:
        void updateComic( const QString &identifierSuffix = QString() );
        void updateButtons();
        void loadConfig();
        void saveConfig();

        QImage mImage;
        QDate mCurrentDate;
        QUrl mWebsiteUrl;
        QString mComicIdentifier;
        QString mNextIdentifierSuffix;
        QString mPreviousIdentifierSuffix;
        ConfigWidget *mConfigWidget;
        bool mScaleComic;
        bool mShowPreviousButton;
        bool mShowNextButton;
};

K_EXPORT_PLASMA_APPLET(comic, ComicApplet)

#endif
