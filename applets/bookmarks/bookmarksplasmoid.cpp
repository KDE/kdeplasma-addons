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

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>
// KDE
#include <KActionCollection>
#include <KIconLoader>
#include <KIcon>
#include <KBookmarkMenu>
#include <KRun>
// Qt
#include <QtGui/QPainter>
#include <QtGui/QGraphicsLinearLayout>


class BookmarkOwner : public KBookmarkOwner
{
  public:
    BookmarkOwner() : KBookmarkOwner() {}

  public: // KBookmarkOwner API
    virtual bool enableOption(BookmarkOption) const { return false; }
    virtual bool supportsTabs() const { return false; }
    virtual void openBookmark( const KBookmark& bookmark, Qt::MouseButtons, Qt::KeyboardModifiers )
    {
        new KRun( bookmark.url(), (QWidget*)0 );
    }
};


namespace Plasma
{

BookmarksPlasmoid::BookmarksPlasmoid( QObject* parent, const QVariantList& args )
  : Applet( parent, args ),
    mIcon( new IconWidget(KIcon( QString::fromLatin1("bookmarks") ),QString(),this) ),
    mBookmarkMenu( 0 ),
    mBookmarkOwner( 0 )
{
    setAspectRatioMode( ConstrainedSquare );
    setHasConfigurationInterface( false );
    setBackgroundHints( NoBackground );

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );
    layout->addItem( mIcon );

    mIcon->setFlag( ItemIsMovable, false );
    connect( mIcon, SIGNAL(pressed( bool )), SLOT(toggleMenu( bool )) );
    connect( this, SIGNAL(activate()), SLOT(toggleMenu()) );

    resize( IconSize(KIconLoader::Desktop) * 2, IconSize(KIconLoader::Desktop) * 2 );
}


void BookmarksPlasmoid::init()
{
    Plasma::ToolTipManager::self()->registerWidget( this );
    Plasma::ToolTipContent toolTipContent( i18n("Bookmarks"), i18n("Quick Access to the Bookmarks"), mIcon->icon() );
    Plasma::ToolTipManager::self()->setContent( this, toolTipContent );
}

void BookmarksPlasmoid::toggleMenu( bool toggle )
{
    if( ! toggle )
        return;

    Plasma::ToolTipManager::self()->hide( this );
    mIcon->setPressed();

    const bool isFirstTime = ( mBookmarkOwner == 0 );
    if( isFirstTime )
    {
        mBookmarkOwner = new BookmarkOwner();
        mActionCollection = new KActionCollection( this );
    }

    delete mBookmarkMenu;

    KBookmarkManager* bookmarkManager = KBookmarkManager::userBookmarksManager();

    KMenu* menu = new KMenu();
    menu->setAttribute( Qt::WA_DeleteOnClose );
    connect( menu, SIGNAL(aboutToHide()), mIcon, SLOT(setUnpressed()) );
    mBookmarkMenu = new KBookmarkMenu( bookmarkManager, mBookmarkOwner, menu, mActionCollection );

    menu->popup( popupPosition(menu->sizeHint()) );
}

void BookmarksPlasmoid::toggleMenu()
{
    toggleMenu( true );
}

BookmarksPlasmoid::~BookmarksPlasmoid()
{
    delete mBookmarkMenu;
    delete mBookmarkOwner;
}

}
