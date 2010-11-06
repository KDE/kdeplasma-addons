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

#include "RecentDocuments.h"
#include <KStandardDirs>
#include <KIcon>
#include <QVariant>

namespace Lancelot {
namespace Models {

RecentDocuments::RecentDocuments()
    : FolderModel(KStandardDirs::locateLocal("data", QLatin1String("RecentDocuments/")), QDir::Time)
{
    setSelfTitle(i18n("Recent Documents"));
    setSelfIcon(KIcon("document-open-recent"));
}

RecentDocuments::~RecentDocuments()
{
}

bool RecentDocuments::hasContextActions(int index) const
{
    Q_UNUSED(index);
    return true;
}

void RecentDocuments::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    if (index > size() - 1) return;

    menu->addAction(KIcon("edit-delete"), i18n("Remove this item"))
        ->setData(QVariant(0));
    menu->addAction(KIcon("edit-clear-history.png"), i18n("Clear documents history"))
        ->setData(QVariant(1));
}

void RecentDocuments::contextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    KUrl url = KUrl(itemAt(index).data.toString());

    int action = context->data().toInt();
    switch (action) {
        case 0:
            // deleting this item
            QFile::remove(url.path());
            break;
        case 1:
            // clearing the list
            foreach (const QString &file, QDir(dirPath()).entryList(QDir::Files)) {
                QFile::remove(dirPath() + '/' + file);
            }
            break;
    }

    update();
}

bool RecentDocuments::dataDropAvailable(int where, const QMimeData * mimeData)
{
    Q_UNUSED(where);
    Q_UNUSED(mimeData);
    return false;
}

} // namespace Models
} // namespace Lancelot

#include "RecentDocuments.moc"
