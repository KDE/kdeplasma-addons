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

#ifndef BOOKMARKSPLASMOID_H
#define BOOKMARKSPLASMOID_H

// Plasma
#include <Plasma/Applet>

class GeneralConfigEditor;
class BookmarkOwner;

class KBookmarkMenu;
class KBookmarkManager;


namespace Plasma
{
class IconWidget;


class BookmarksPlasmoid : public Applet
{
    Q_OBJECT

public:
    BookmarksPlasmoid(QObject* parent, const QVariantList& args);

    virtual ~BookmarksPlasmoid();

public: // Plasma::Applet API
    virtual void init();
    virtual QList<QAction*> contextualActions();
    virtual void createConfigurationInterface(KConfigDialog* parent);

public Q_SLOTS:
    virtual void configChanged();

protected:
    void updateFolderData();

protected Q_SLOTS:
    void toggleMenu(bool toggle);
    void toggleMenu();
    void editBookmarks();
    void applyConfigChanges();

    void onBookmarksChanged(const QString& address);

private:
    QString mBookmarkFolderAddress;

    IconWidget* mIcon;

    QList<QAction*> mContextualActions;

    KBookmarkManager* mBookmarkManager;
    KBookmarkMenu* mBookmarkMenu;
    BookmarkOwner* mBookmarkOwner;

    GeneralConfigEditor* mGeneralConfigEditor;
};

}

#endif
