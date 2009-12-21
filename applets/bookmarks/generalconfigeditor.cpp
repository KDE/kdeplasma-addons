/*
    This file is part of the Bookmarks plasmoid, part of the KDE project.

    Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "generalconfigeditor.h"

// KDE
#include <KBookmarkManager>
// #include <KBookmarkDialog> TODO: CamelCase only added after 4.3.4 and 4.5.0 Beta 2
#include <kbookmarkdialog.h>
#include <KSqueezedTextLabel>
#include <KPushButton>
#include <KLocale>
#include <KIconLoader>
// Qt
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>


GeneralConfigEditor::GeneralConfigEditor( KBookmarkManager* bookmarkManager, QWidget* parent )
  : QWidget( parent ),
    mBookmarkFolder( bookmarkManager->root() ),
    mBookmarkManager( bookmarkManager )
{
    QVBoxLayout* pageLayout = new QVBoxLayout( this );
    pageLayout->setMargin( 0 );

    // folder selection
    QHBoxLayout* folderLayout = new QHBoxLayout;
    folderLayout->setMargin( 0 );

    const QString folderSelectLabelText =
         i18nc( "@label:edit the bookmark folder to show",
                "Folder:" );
    QLabel* label = new QLabel( folderSelectLabelText );
    mFolderIconLabel = new QLabel;
    mFolderNameLabel = new KSqueezedTextLabel;
    KPushButton* folderSelectButton = new KPushButton( KIcon("folder-open"), QString() );
    label->setBuddy( folderSelectButton );
    connect( folderSelectButton, SIGNAL(clicked( bool )), SLOT(selectBookmarkFolder()) );

    const QString folderToolTip =
        i18nc( "@info:tooltip",
               "The folder which will be used as the base for the menu." );
    label->setToolTip( folderToolTip );
    folderSelectButton->setToolTip( folderToolTip );
//     const QString folderWhatsThis =
//         i18nc( "@info:whatsthis",
//                "Select the folder which should be used to in the menu." );
//     folderSelectButton->setWhatsThis( groupSizeWhatsThis );

    folderLayout->addWidget( label );
    folderLayout->addWidget( mFolderIconLabel );
    folderLayout->addWidget( mFolderNameLabel, 10 );
    folderLayout->addWidget( folderSelectButton );

    pageLayout->addLayout( folderLayout );
    pageLayout->addStretch();

    updateFolder();
}

void GeneralConfigEditor::setBookmarkFolderAddress( const QString& bookmarkFolderAddress )
{
    KBookmark bookmark = mBookmarkManager->findByAddress( bookmarkFolderAddress );

    if( bookmark.isNull() || ! bookmark.isGroup() )
        return;

    mBookmarkFolder = bookmark.toGroup();

    updateFolder();
}

void GeneralConfigEditor::selectBookmarkFolder()
{
    KBookmarkDialog* dialog = new KBookmarkDialog( mBookmarkManager, this );
    KBookmarkGroup selectedFolder = dialog->selectFolder( mBookmarkFolder );
    if( ! selectedFolder.isNull() ) // TODO: would isValid be better? API dox are inconsistent
    {
        mBookmarkFolder = selectedFolder;
        updateFolder();
    }
    delete dialog;
}

void GeneralConfigEditor::updateFolder()
{
    const bool isRoot = ( ! mBookmarkFolder.hasParent() );

    mFolderIconLabel->setPixmap( SmallIcon(isRoot?QString::fromLatin1("bookmarks"):mBookmarkFolder.icon()) );
    mFolderNameLabel->setText( isRoot?i18n("Bookmarks"):mBookmarkFolder.text() );
    // TODO: ellipsis label and tooltip
}
