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

#include "RecentDocuments.h"
#include <KStandardDirs>
#include <KIcon>
#include <QVariant>

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
            foreach (QString file, QDir(m_dirPath).entryList(QDir::Files)) {
                QFile::remove(m_dirPath + '/' + file);
            }
            break;
    }

    m_dirLister->updateDirectory(KUrl(m_dirPath));
}

} // namespace Models

#include "RecentDocuments.moc"
