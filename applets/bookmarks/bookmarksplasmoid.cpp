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

#include "bookmarksplasmoid.h"

// Plasmoid
#include "generalconfigeditor.h"
#include "bookmarkowner.h"
// Plasma
#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>
// KDE
#include <KConfigDialog>
#include <KStandardAction>
#include <KIconLoader>
#include <KIcon>
#include <KBookmarkMenu>
#include <KBookmarkGroup>
// Qt
#include <QtGui/QPainter>
#include <QtGui/QGraphicsLinearLayout>


namespace Plasma
{

static const char bookmarkFolderAddressConfigKey[] = "BookmarkFolderAddress";


BookmarksPlasmoid::BookmarksPlasmoid(QObject* parent, const QVariantList& args)
  : Applet(parent, args),
    mIcon(0),
    mBookmarkManager(0),
    mBookmarkMenu(0),
    mBookmarkOwner(0)
{
}


void BookmarksPlasmoid::init()
{
    mBookmarkManager = KBookmarkManager::userBookmarksManager();
    mBookmarkManager->setEditorOptions(name(), true);
    connect(mBookmarkManager, SIGNAL(changed(QString,QString)), SLOT(onBookmarksChanged(QString)));

    // general
    setHasConfigurationInterface(true);
    connect(this, SIGNAL(activate()), SLOT(toggleMenu()));
    Plasma::ToolTipManager::self()->registerWidget(this);

    // context menu
    KAction* editorOpener = KStandardAction::editBookmarks(this, SLOT(editBookmarks()), this);
    mContextualActions.append(editorOpener);

    // view
    setAspectRatioMode(ConstrainedSquare);
    setBackgroundHints(NoBackground);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mIcon = new IconWidget(KIcon("bookmarks"),"",this);
    mIcon->setFlag(ItemIsMovable, false);
    connect(mIcon, SIGNAL(pressed(bool)), SLOT(toggleMenu(bool)));
    layout->addItem(mIcon);


    configChanged();
}

QList<QAction*> BookmarksPlasmoid::contextualActions()
{
    return mContextualActions;
}

void BookmarksPlasmoid::updateFolderData()
{
    const KBookmark bookmark = mBookmarkManager->findByAddress(mBookmarkFolderAddress);

    KBookmarkGroup bookmarkFolder =
        (bookmark.isNull() || ! bookmark.isGroup()) ? mBookmarkManager->root() : bookmark.toGroup();

    const bool isRoot = (! bookmarkFolder.hasParent());

    const QString iconName = isRoot ? QString::fromLatin1("bookmarks") : bookmarkFolder.icon();
    const QString folderName = isRoot ? i18nc("name of the container of all browser bookmarks",
                                              "Bookmarks") :
                                        bookmarkFolder.text();
    const QString comment = isRoot ? i18n("Quick access to your bookmarks.") : bookmarkFolder.description();

    // icon
    mIcon->setIcon(iconName);
    // tooltip
    Plasma::ToolTipContent toolTipContent(folderName, comment, KIcon(iconName));
    Plasma::ToolTipManager::self()->setContent(this, toolTipContent);
}

void BookmarksPlasmoid::toggleMenu(bool toggle)
{
    if (! toggle)
        return;

    Plasma::ToolTipManager::self()->hide(this);
    mIcon->setPressed();

    const bool isFirstTime = (mBookmarkOwner == 0);
    if (isFirstTime)
        mBookmarkOwner = new BookmarkOwner();

    delete mBookmarkMenu;

    KMenu* menu = new KMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    connect(menu, SIGNAL(aboutToHide()), mIcon, SLOT(setUnpressed()));
    // TODO: only renew if manager emits changed
    mBookmarkMenu = new KBookmarkMenu(mBookmarkManager, mBookmarkOwner, menu, mBookmarkFolderAddress);

    menu->popup(popupPosition(menu->size()));
    menu->move(popupPosition(menu->size()));

}

void BookmarksPlasmoid::toggleMenu()
{
    toggleMenu(true);
}

void BookmarksPlasmoid::createConfigurationInterface(KConfigDialog* parent)
{
    mGeneralConfigEditor = new GeneralConfigEditor(mBookmarkManager, parent);
    mGeneralConfigEditor->setBookmarkFolderAddress(mBookmarkFolderAddress);
    const QString pageName = i18nc("@title:tab name of settings page with general parameters","General");
    parent->addPage(mGeneralConfigEditor, pageName, icon());
    connect(parent, SIGNAL(applyClicked()), SLOT(applyConfigChanges()));
    connect(parent, SIGNAL(okClicked()), SLOT(applyConfigChanges()));
}

void BookmarksPlasmoid::applyConfigChanges()
{
    const QString& bookmarkFolderAddress = mGeneralConfigEditor->bookmarkFolderAddress();

    if (mBookmarkFolderAddress != bookmarkFolderAddress) {
        KConfigGroup configGroup = config();
        configGroup.writeEntry(bookmarkFolderAddressConfigKey, bookmarkFolderAddress);
        emit configNeedsSaving();
    }
}

void BookmarksPlasmoid::configChanged()
{
    // read config
    KConfigGroup configGroup = config();
    const QString bookmarkFolderAddress = configGroup.readEntry(bookmarkFolderAddressConfigKey, mBookmarkFolderAddress);

    if (mBookmarkFolderAddress != bookmarkFolderAddress) {
        mBookmarkFolderAddress = bookmarkFolderAddress;
        updateFolderData();
    }
}

void BookmarksPlasmoid::editBookmarks()
{
    mBookmarkManager->slotEditBookmarksAtAddress(mBookmarkFolderAddress);
}

void BookmarksPlasmoid::onBookmarksChanged(const QString& address)
{
    Q_UNUSED(address);

    updateFolderData();
}

BookmarksPlasmoid::~BookmarksPlasmoid()
{
    delete mBookmarkMenu;
    delete mBookmarkOwner;
}

}
