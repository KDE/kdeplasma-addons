/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "RecentDocuments.h"
#include <KStandardDirs>
#include <KIcon>
#include <QVariant>

RecentDocuments::RecentDocuments()
    : FolderModel()
{
    setPath(KStandardDirs::locateLocal("data", QLatin1String("RecentDocuments/")));
    // TODO: setSorting(QDir::Time);

    setSelfTitle(i18n("Recent Documents"));
    setSelfIcon(KIcon("document-open-recent"));
}

RecentDocuments::~RecentDocuments()
{
}

#include "RecentDocuments.moc"
