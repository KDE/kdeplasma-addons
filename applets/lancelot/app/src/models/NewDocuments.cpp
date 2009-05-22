/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "NewDocuments.h"
#include <KStandardDirs>
#include <KIcon>

namespace Models {

NewDocuments::NewDocuments()
    : FolderModel(NewDocuments::path(), QDir::Name)
{
    setSelfTitle(i18n("New:"));
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
                    QFile::copy(
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
