/*
 *   Copyright 2009 Andrew Stromme  <astromme@chatonka.com>
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

#include "listsource.h"
#include <rtm/task.h>
#include <rtm/session.h>

ListSource::ListSource(RTM::ListId i, RTM::Session *s, QObject* parent)
  : DataContainer(parent),
  id(i),
  session(s),
  list(0)
{
  connect(this, SIGNAL(updateRequested(DataContainer*)), SLOT(updateRequest(DataContainer*)));
  setObjectName("List:" + QString::number(i));
  update();
}

ListSource::~ListSource() {

}


void ListSource::updateRequest(Plasma::DataContainer* source) {
  if (source == this)
    update();
}

void ListSource::update() {
  if (!list) {
    list = session->listFromId(id);
    if (!list)
      return;
  }
  
  removeAllData();
  
  setData("name", list->name());
  setData("id", list->id());
  setData("smart", list->isSmart());
  setData("filter", list->filter());
  
  foreach(RTM::Task *task, list->tasks)
    setData(QString::number(task->id()), task->name());
    
  checkForUpdate();
}

