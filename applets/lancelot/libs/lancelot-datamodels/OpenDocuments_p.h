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

#ifndef LANCELOTAPP_MODELS_OPENDOCUMENTS_P_H
#define LANCELOTAPP_MODELS_OPENDOCUMENTS_P_H

#include "OpenDocuments.h"
#include <taskmanager/taskmanager.h>
#include <QSet>

using TaskManager::TaskPtr;

namespace Lancelot {
namespace Models {

class SupportedTask {
public:
    explicit SupportedTask(const QString & classPattern = 0,
            const QString & documentNameExtractor = 0);
    QRegExp m_classPattern;
    QRegExp m_documentNameExtractor;
};

class OpenDocuments::Private : public QObject {
    Q_OBJECT
public:
    Private(OpenDocuments * parent);

public Q_SLOTS:
    void taskChanged();
    void taskAdded(TaskPtr task);
    void taskRemoved(TaskPtr task);

    void connectTask(TaskPtr task);
    bool setDataForTask(TaskPtr task);

    int indexOf(WId wid);

public:
    QMap <WId, TaskPtr > tasks;
    QList <SupportedTask> supportedTasks;

private:
    OpenDocuments * const q;

};


} // namespace Models
} // namespace Lancelot

#endif /* LANCELOTAPP_MODELS_OPENDOCUMENTS_H */
