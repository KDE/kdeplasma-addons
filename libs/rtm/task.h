/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
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

#ifndef RTM_TASK_H
#define RTM_TASK_H

// Qt Includes
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QString>
#include <QObject>

// Local Includes
#include "rtm.h"
#include "note.h"
#include <QStringList>

namespace RTM {

class TaskPrivate;
class Session;

class RTM_EXPORT Task : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QStringList tags READ tags WRITE setTags)
  Q_PROPERTY(qulonglong listId READ listId WRITE setList)
  Q_PROPERTY(QDateTime completedTime READ completed)
  Q_PROPERTY(bool isCompleted READ isCompleted WRITE setCompleted)
  Q_PROPERTY(bool isDeleted READ isDeleted WRITE setDeleted)
  Q_PROPERTY(QDateTime deletedTime READ deleted)
  Q_PROPERTY(int priority READ priority WRITE setPriority)
  Q_PROPERTY(QDateTime due READ due WRITE setDue)
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(qulonglong seriesId READ seriesId)
  Q_PROPERTY(QString estimate READ estimate WRITE setEstimate)
  Q_PROPERTY(qulonglong locationId READ locationId WRITE setLocationId)
  Q_PROPERTY(QString repeatString READ repeatString WRITE setRepeatString)
  Q_PROPERTY(QString url READ url WRITE setUrl)
  Q_PROPERTY(qulonglong id READ id)
  //Q_PROPERTY(Notes notes READ notes WRITE setNotes)

  public:
    virtual ~Task();

    static Task* uninitializedTask(RTM::Session* session);

    QStringList tags() const;
    RTM::ListId listId() const;
    QDateTime completed() const;
    bool isCompleted() const;
    bool isDeleted() const;
    QDateTime deleted() const;
    int priority() const;
    QDateTime due() const;
    QString name() const;
    RTM::TaskSeriesId seriesId() const;
    QString estimate() const;
    LocationId locationId() const;
    QString repeatString() const;
    QString url() const;
    RTM::TaskId id() const;
    Notes notes() const;

    void setTags(const QStringList &tags);
    void setList(ListId listId);
    void setCompleted(bool completed);
    void setDeleted(bool deleted);
    void setPriority(int priority);
    void setDue(const QDateTime& dueDate);
    void setDue(const QString& date);
    void setName(const QString& name);
    void setEstimate(const QString& estimate);
    void setLocationId(LocationId locationid);
    void setRepeatString(const QString& repeatString);
    void setUrl(const QString& url);
    void setNotes(const Notes& notes);

    void addTag(const Tag& tag);
    bool removeTag(const Tag& tag);
    void removeAllTags();
    Q_INVOKABLE void removeDue();

    void addNote(const QString& title, const QString& text);
    bool editNote(RTM::NoteId noteid, const QString& newTitle, const QString& newText);
    bool removeNote(RTM::NoteId noteid);
    void removeAllNotes();

    void postpone();

    int increasePriority(); // Returns priority
    int decreasePriority();

    void undoLastAction();

  private:
    Task(RTM::Session* session);

    friend class TasksReader;
    friend class TaskPrivate;
    TaskPrivate * const d;
};

} // namespace RTM

#endif
