/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "lancelotcomponentsdataplugin.h"

#include <QtDeclarative/qdeclarative.h>

#include "FavoriteApplications.h"
#include "FolderModel.h"
#include "DirModel.h"

#define QML_REGISTER_TYPE(Type) qmlRegisterType < Type > (uri, 0, 1, #Type)

void LancelotComponentsDataPlugin::registerTypes(const char * uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.lancelot.components.data"));

    QML_REGISTER_TYPE(FavoriteApplications);
    QML_REGISTER_TYPE(FolderModel);
    QML_REGISTER_TYPE(DirModel);
}


#include "lancelotcomponentsdataplugin.moc"

