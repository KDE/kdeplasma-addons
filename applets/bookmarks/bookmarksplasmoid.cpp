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
    connect(mBookmarkManager, SIGNAL(changed(const QString&, const QString&)), SLOT(onBookmarksChanged(const QString&)));

    // read config
    KConfigGroup configGroup = config();
    mBookmarkFolderAddress = configGroup.readEntry(bookmarkFolderAddressConfigKey);

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

    mIcon = new IconWidget(this);
    mIcon->setFlag(ItemIsMovable, false);
    connect(mIcon, SIGNAL(pressed(bool)), SLOT(toggleMenu(bool)));
    layout->addItem(mIcon);

    resize(IconSize(KIconLoader::Desktop) * 2, IconSize(KIconLoader::Desktop) * 2);

    updateFolderData();
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
    const QString folderName = isRoot ? i18n("Bookmarks") : bookmarkFolder.text();
    QString comment;
    if (isRoot)
        comment = i18n("Quick access to your bookmarks.");
    else {
        const QDomNode subnode = bookmarkFolder.internalElement().namedItem("desc");
        comment = (subnode.firstChild().isNull()) ? QString() : subnode.firstChild().toText().data();
    }

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

    menu->popup(popupPosition(menu->sizeHint()));
}

void BookmarksPlasmoid::toggleMenu()
{
    toggleMenu(true);
}

void BookmarksPlasmoid::createConfigurationInterface(KConfigDialog* parent)
{
    mGeneralConfigEditor = new GeneralConfigEditor(mBookmarkManager, parent);
    mGeneralConfigEditor->setBookmarkFolderAddress(mBookmarkFolderAddress);
    parent->addPage(mGeneralConfigEditor, i18n("General"), icon());
    connect(parent, SIGNAL(applyClicked()), SLOT(applyConfigChanges()));
    connect(parent, SIGNAL(okClicked()), SLOT(applyConfigChanges()));
}

void BookmarksPlasmoid::applyConfigChanges()
{
    const QString& bookmarkFolderAddress = mGeneralConfigEditor->bookmarkFolderAddress();

    if (mBookmarkFolderAddress != bookmarkFolderAddress) {
        mBookmarkFolderAddress = bookmarkFolderAddress;

        KConfigGroup configGroup = config();
        configGroup.writeEntry(bookmarkFolderAddressConfigKey, mBookmarkFolderAddress);

        emit configNeedsSaving();

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
