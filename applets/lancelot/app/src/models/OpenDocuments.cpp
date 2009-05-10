/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
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

#include "OpenDocuments.h"
#include <KIcon>
#include <QDebug>

namespace Models {

OpenDocuments::SupportedTask::SupportedTask(const QString & classPattern,
        const QString & documentNameExtractor)
    : m_classPattern(classPattern), m_documentNameExtractor(documentNameExtractor)
{
}

OpenDocuments::OpenDocuments()
{
    setSelfTitle(i18nc("@title Title of a list of documents that are open",
            "Open documents"));
    setSelfIcon(KIcon("document-edit"));

    m_supportedTasks
        // KDE applications
        << SupportedTask("(kate|kwrite|kword|krita|karbon|kchart|kexi|kformula|kpresenter|kspread).*", ".*([^/]+) . ([^ ]*)")

        // OpenOffice.org
        << SupportedTask("VCLSalFrame.*", "([^-]+) - ([^-]*)")

        // Other
        << SupportedTask("gimp.*", "([^-]+) . ([^-]*)")
        << SupportedTask("inkscape.*", "([^-]+) - ([^-]*)")
        << SupportedTask("gvim.*", "([^-]+) [(][^)]*[)] - ([^-]*)")
        ;

    load();
}

OpenDocuments::~OpenDocuments()
{
}

void OpenDocuments::connectTask(TaskPtr task)
{
    Q_ASSERT(task);
    connect(
        task.constData(), SIGNAL(changed(::TaskManager::TaskChanges)),
        this, SLOT(taskChanged())
    );
}

void OpenDocuments::load()
{
    foreach (TaskPtr task, TaskManager::TaskManager::self()->tasks().values()) {
        if (setDataForTask(task)) {
            connectTask(task);
        }
    }

    connect(
        TaskManager::TaskManager::self(), SIGNAL(taskAdded(TaskPtr)),
        this, SLOT(taskAdded(TaskPtr))
    );

    connect(
        TaskManager::TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)),
        this, SLOT(taskRemoved(TaskPtr))
    );
}

void OpenDocuments::taskAdded(TaskPtr task)
{
    connectTask(task);
    setDataForTask(task);
}

void OpenDocuments::taskRemoved(TaskPtr task)
{
    Q_ASSERT(task);
    int index = indexOf(task->window());
    if (index != -1) {
        removeAt(index);
        m_tasks.remove(task->window());
    }
}

void OpenDocuments::taskChanged()
{
    TaskManager::Task* task = qobject_cast<TaskManager::Task*>(sender());
    Q_ASSERT(task);

    setDataForTask(TaskPtr(task));
}

bool OpenDocuments::setDataForTask(TaskPtr task)
{
    Q_ASSERT(task);

    QRegExp extractor;
    QString className = task->className();

    foreach (const SupportedTask &st, m_supportedTasks) {
        // qDebug() << "OpenDocuments::setDataForTask" << task->className() << task->visibleName();
        if (st.m_classPattern.exactMatch(task->className())) {
            extractor = st.m_documentNameExtractor;
            break;
        }
    }
    if (extractor.isEmpty()) {
        return false;
    }

    int index = indexOf(task->window());
    if (index == -1) {
        index = size();
        add (
            "", "", QIcon(), uint(task->window())
        );
        m_tasks[task->window()] = task;
    }

    QString title = task->visibleName();
    QString description;
    if (extractor.exactMatch(title)) {
        title = extractor.cap(1);
        description = extractor.cap(2);
    }

    QIcon icon = QIcon(task->icon(32, 32));

    set(index, title, description, icon, uint(task->window()));

    return true;
}

int OpenDocuments::indexOf(WId wid)
{
    for (int i = size() - 1; i >= 0; i--) {
        const Item * item = & itemAt(i);
        if (item->data.toUInt() == wid) {
            return i;
        }
    }

    /*
    QListIterator<Item> i(m_items);
    int index = 0;

    while (i.hasNext()) {
        const Item & item = i.next();
        if (item.data.toUInt() == wid) {
            return index;
        }
        ++index;
    }
    */

    return -1;
}

void OpenDocuments::activate(int index)
{
    bool valid = true;
    WId wid = itemAt(index).data.toUInt(&valid);
    if (valid && m_tasks.contains(wid)) {
        m_tasks[wid]->activate();
    }
    hideLancelotWindow();
}

} // namespace Models

#include "OpenDocuments.moc"
