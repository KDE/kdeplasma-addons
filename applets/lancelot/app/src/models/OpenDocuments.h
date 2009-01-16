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

#ifndef LANCELOTAPP_MODELS_OPENDOCUMENTS_H
#define LANCELOTAPP_MODELS_OPENDOCUMENTS_H

#include "BaseModel.h"
#include <taskmanager/taskmanager.h>
#include <QSet>

using TaskManager::TaskPtr;

namespace Models {

class OpenDocuments : public BaseModel {
    Q_OBJECT
public:
    OpenDocuments();
    virtual ~OpenDocuments();

protected:
    void activate(int index);
    void load();

private Q_SLOTS:
    void taskChanged();
    void taskAdded(TaskPtr task);
    void taskRemoved(TaskPtr task);

private:
    class SupportedTask {
    public:
        explicit SupportedTask(const QString & classPattern = 0,
                const QString & documentNameExtractor = 0);
        QRegExp m_classPattern;
        QRegExp m_documentNameExtractor;
    };

    void connectTask(TaskPtr task);
    bool setDataForTask(TaskPtr task);

    int indexOf(WId wid);

    QMap <WId, TaskPtr > m_tasks;
    QList <SupportedTask> m_supportedTasks;
};

} // namespace Models

#endif /* LANCELOTAPP_MODELS_OPENDOCUMENTS_H */
