/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
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

#ifndef COMIC_ARCHIVE_DIALOG_H
#define COMIC_ARCHIVE_DIALOG_H

#include "comicinfo.h"

#include "ui_comicarchivedialog.h"

#include <QDialog>

class ComicArchiveDialog : public QDialog
{
    Q_OBJECT

    public:
        ComicArchiveDialog( const QString &pluginName, const QString &comicName, IdentifierType identifierType, const QString &currentIdentifierSuffix, const QString &firstIdentifierSuffix, const QString &savingDir = QString(), QWidget *parent = nullptr );

    Q_SIGNALS:
        void archive( int archiveType, const QUrl &dest, const QString &fromIdentifier, const QString &toIdentifier );

    private Q_SLOTS:
        void archiveTypeChanged( int newType );
        void fromDateChanged( const QDate &newDate );
        void toDateChanged( const QDate &newDate );
        void slotOkClicked();
        void updateOkButton();

    private:
        void setFromVisible( bool visible );
        void setToVisibile( bool visible );

    private:
        Ui::ComicArchiveDialog ui;
        IdentifierType mIdentifierType;
        QString mPluginName;
};

#endif
