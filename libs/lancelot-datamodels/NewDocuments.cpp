/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "NewDocuments.h"

#include <KDebug>
#include <KStandardDirs>
#include <KIcon>

namespace Lancelot {
namespace Models {

#define LinkOrCopy(source, destination)           \
    kDebug() << "Models::LinkOrCopy:"             \
                 << source << destination;        \
    if (!QFile::link((source), (destination))) {  \
        kDebug() << "Models::LinkOrCopy:"         \
                 << "Linking failed ... copying"; \
        QFile::copy((source), (destination));     \
    }

NewDocuments::NewDocuments()
    : FolderModel(NewDocuments::path(), QDir::Name)
{
    setSelfTitle(i18nc("New documents", "New:"));
    setSelfIcon(KIcon("document-new"));

    QDir dir(path());
    if (dir.entryList(QDir::Files | QDir::NoDotAndDotDot).size() == 0) {

        QStringList applications;
        applications
            << "ooo-writer|writer|kword"
            << "ooo-impress|impress|kpresenter"
            << "ooo-calc|calc|kspread"
            << "inkscape|karbon"
            << "gimp|krita";

        int index = 0;

        foreach (const QString & serviceAlternatives, applications) {
            foreach (const QString & serviceName, serviceAlternatives.split('|')) {
                const KService::Ptr service = KService::serviceByStorageId(serviceName);
                if (service) {
                    QFileInfo file(service->entryPath());
                    LinkOrCopy(
                        service->entryPath(),
                        dir.absolutePath() + '/' +
                        QString::number(index++) + '_' + file.fileName());
                    break;
                }
            }
        }
    }
}

NewDocuments::~NewDocuments()
{
}

QString NewDocuments::path()
{
    QString path = KStandardDirs::locateLocal("data", "lancelot", true);
    if (!path.endsWith('/')) {
        path += '/';
    }
    path += "/newdocuments/";

    QDir dir;
    dir.mkpath(path);

    return path;
}

} // namespace Models
} // namespace Lancelot
