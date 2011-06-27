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

#include "listssource.h"

#include <rtm/rtm.h>
#include <rtm/list.h>
#include <rtm/session.h>

#include <KDebug>
#include <Plasma/DataEngine>

#include "rtmengine.h"

ListsSource::ListsSource(RtmEngine* engine, RTM::Session* session)
    : Plasma::DataContainer(engine),
      m_engine(engine),
      m_session(session)
{
  // Session should be an authenticated RTM Session.
  connect(session, SIGNAL(listsChanged()), this, SLOT(listsChanged()));
  connect(session, SIGNAL(listChanged(RTM::List*)), this, SLOT(listChanged(RTM::List*)));
  connect(&timer, SIGNAL(timeout()), this, SLOT(refresh()));
  timer.setInterval(1000*60*5); // 5 minute refresh. TODO: Make Configurable.
  timer.start();
  setObjectName("Lists");
  loadCache();
}

ListsSource::~ListsSource()
{
}

ListSource* ListsSource::setupListSource(const QString& source) {
  QString id = source;
  ListSource *listsource = new ListSource(id.remove("List:").toLongLong(), m_session, this);
  return listsource;
}

void ListsSource::refresh()
{
  kDebug() << "Updating Lists";
  if (!m_session->authenticated())
    return; // We can't do anything with a non-authenticated session
  m_session->refreshListsFromServer();
}

void ListsSource::listChanged(RTM::List* list) {
  setData(QString::number(list->id()), list->name());
  m_engine->updateListSource(QString::number(list->id()));
}


void ListsSource::listsChanged() {
  removeAllData();
  loadCache();

  checkForUpdate();
}

void ListsSource::loadCache() {
  foreach(RTM::List *list, m_session->cachedLists()) {
    setData(QString::number(list->id()), list->name());
  }
}

#include "listssource.moc"
