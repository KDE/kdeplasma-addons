/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "FolderModel.h"
#include <KDirWatch>
#include <QDir>
#include <KDebug>

namespace Lancelot {
namespace Models {

KDirWatch * FolderModel::m_dirWatch = NULL;

FolderModel::FolderModel(QString dirPath, QDir::SortFlags sort)
    : m_dirPath(dirPath)
{
    if (!m_dirPath.endsWith(QDir::separator())) {
        m_dirPath += QDir::separator();
    }

    if (m_dirWatch == NULL) {
        m_dirWatch = new KDirWatch();
    }

    kDebug() << "Watching " << m_dirPath;
    m_dirWatch->addDir(m_dirPath);

    connect (m_dirWatch, SIGNAL(dirty(const QString &)), this, SLOT(dirty(const QString &)));

    load();
}

FolderModel::~FolderModel()
{
}

void FolderModel::load()
{
    m_items.clear();

    kDebug() << m_dirPath;

    QStringList files = QDir(m_dirPath).entryList(
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name
    );

    foreach (const QString & file, files) {
        addUrl(m_dirPath + file);
    }
}

void FolderModel::dirty(const QString & dirPath)
{
    kDebug() << dirPath << " is dirty and we want " << m_dirPath;
    if (m_dirPath == dirPath) {
        load();
    }
}

}
}

#include "FolderModel.moc"
