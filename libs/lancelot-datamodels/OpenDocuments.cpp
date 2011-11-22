/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
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

#include "OpenDocuments.h"
#include "OpenDocuments_p.h"
#include <KIcon>
#include <KDebug>

namespace Lancelot {
namespace Models {

SupportedTask::SupportedTask(const QString & classPattern,
        const QString & documentNameExtractor)
    : m_classPattern(classPattern), m_documentNameExtractor(documentNameExtractor)
{
}

OpenDocuments::Private::Private(OpenDocuments * parent)
    : q(parent)
{
}

OpenDocuments::OpenDocuments()
    : d(new Private(this))
{
    setSelfTitle(i18nc("@title Title of a list of documents that are open",
            "Open documents"));
    setSelfIcon(KIcon("document-edit"));

    d->supportedTasks
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
    kDebug() << "deleting...";
    delete d;
}

void OpenDocuments::Private::connectTask(::TaskManager::Task *task)
{
    Q_ASSERT(task);
    connect(
        task, SIGNAL(changed(::TaskManager::TaskChanges)),
        this, SLOT(taskChanged())
    );
}

void OpenDocuments::load()
{
    foreach (TaskManager::Task *task, TaskManager::TaskManager::self()->tasks()) {
        if (d->setDataForTask(task)) {
            d->connectTask(task);
        }
    }

    connect(
        TaskManager::TaskManager::self(), SIGNAL(taskAdded(::TaskManager::Task*)),
        d, SLOT(taskAdded(::TaskManager::Task*))
    );

    connect(
        TaskManager::TaskManager::self(), SIGNAL(taskRemoved(::TaskManager::Task*)),
        d, SLOT(taskRemoved(::TaskManager::Task*))
    );
}

void OpenDocuments::Private::taskAdded(::TaskManager::Task *task)
{
    connectTask(task);
    setDataForTask(task);
}

void OpenDocuments::Private::taskRemoved(::TaskManager::Task *task)
{
    Q_ASSERT(task);
    int index = indexOf(task->window());
    if (index != -1) {
        q->removeAt(index);
        tasks.remove(task->window());
    }
}

void OpenDocuments::Private::taskChanged()
{
    TaskManager::Task* task = qobject_cast<TaskManager::Task*>(sender());
    Q_ASSERT(task);

    setDataForTask(task);
}

bool OpenDocuments::Private::setDataForTask(::TaskManager::Task *task)
{
    Q_ASSERT(task);

    QRegExp extractor;
    QString className = task->className();

    foreach (const SupportedTask &st, supportedTasks) {
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
        index = q->size();
        q->add (
            "", "", QIcon(), uint(task->window())
        );
        tasks[task->window()] = task;
    }

    QString title = task->visibleName();
    QString description;
    if (extractor.exactMatch(title)) {
        title = extractor.cap(1);
        description = extractor.cap(2);
    }

    QIcon icon = QIcon(task->icon(32, 32));

    q->set(index, title, description, icon, uint(task->window()));

    return true;
}

int OpenDocuments::Private::indexOf(WId wid)
{
    for (int i = q->size() - 1; i >= 0; i--) {
        const Item * item = & q->itemAt(i);
        if (item->data.toUInt() == wid) {
            return i;
        }
    }

    return -1;
}

void OpenDocuments::activate(int index)
{
    bool valid = true;
    WId wid = itemAt(index).data.toUInt(&valid);
    if (valid && d->tasks.contains(wid)) {
        d->tasks[wid]->activate();
    }
    hideApplicationWindow();
}

} // namespace Models
} // namespace Lancelot

#include "OpenDocuments.moc"
#include "OpenDocuments_p.moc"
