/*
    This file is part of the Bookmarks plasmoid, part of the KDE project.

    Copyright 2009-2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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
#include <KBookmarkDialog>
#include <KBookmarkGroup>
#include <KPushButton>
#include <KLocale>
// Qt
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>


GeneralConfigEditor::GeneralConfigEditor(KBookmarkManager* bookmarkManager, QWidget* parent)
  : QWidget(parent),
    mBookmarkFolderAddress(bookmarkManager->root().address()),
    mBookmarkManager(bookmarkManager)
{
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setMargin(0);

    // folder selection
    QHBoxLayout* folderSelectLayout = new QHBoxLayout;

    const QString folderSelectLabelText =
        i18nc("@label:edit the bookmark folder to show",
              "Folder:");
    QLabel* label = new QLabel(folderSelectLabelText);

    mFolderSelectButton = new KPushButton;
    label->setBuddy(mFolderSelectButton);
    connect(mFolderSelectButton, SIGNAL(clicked(bool)), SLOT(selectBookmarkFolder()));

    const QString folderToolTip =
        i18nc("@info:tooltip",
              "The folder which will be used as the base for the menu.");
    label->setToolTip(folderToolTip);
    mFolderSelectButton->setToolTip(folderToolTip);
//     const QString folderWhatsThis =
//         i18nc( "@info:whatsthis",
//                "Select the folder which should be used to in the menu." );
//     mFolderSelectButton->setWhatsThis( groupSizeWhatsThis );

    folderSelectLayout->addWidget(label);
    folderSelectLayout->addWidget(mFolderSelectButton);
    folderSelectLayout->addStretch();

    pageLayout->addLayout(folderSelectLayout);
    pageLayout->addStretch();

    connect(mBookmarkManager, SIGNAL(changed(QString,QString)), SLOT(onBookmarksChanged(QString)));
    connect(mFolderSelectButton, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));

    updateFolder();
}

void GeneralConfigEditor::setBookmarkFolderAddress(const QString& bookmarkFolderAddress)
{
    if (mBookmarkFolderAddress == bookmarkFolderAddress)
        return;

    mBookmarkFolderAddress = bookmarkFolderAddress;

    updateFolder();
}

void GeneralConfigEditor::selectBookmarkFolder()
{
    const KBookmark bookmarkFolder = mBookmarkManager->findByAddress(mBookmarkFolderAddress);

    KBookmarkDialog* dialog = new KBookmarkDialog(mBookmarkManager, this);
    KBookmarkGroup selectedFolder = dialog->selectFolder(bookmarkFolder);
    delete dialog;

    if (! selectedFolder.isNull()) {
        mBookmarkFolderAddress = selectedFolder.address();
        updateFolder();
    }
}

void GeneralConfigEditor::updateFolder()
{
    const KBookmark bookmarkFolder = mBookmarkManager->findByAddress(mBookmarkFolderAddress);

    const bool isRoot = (! bookmarkFolder.hasParent());

    const QString iconName = isRoot ? QString::fromLatin1("bookmarks") : bookmarkFolder.icon();
    const QString folderName = isRoot ? i18nc("name of the container of all browser bookmarks",
                                              "Bookmarks") :
                                        bookmarkFolder.text();

    mFolderSelectButton->setIcon(KIcon(iconName));
    mFolderSelectButton->setText(folderName);
}


void GeneralConfigEditor::onBookmarksChanged(const QString& address)
{
    Q_UNUSED(address);

    updateFolder();
}
