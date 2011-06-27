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

#ifndef RTMENGINE_H
#define RTMENGINE_H

#include <Plasma/DataEngine>

#include <rtm/rtm.h>

class TaskSource;
namespace RTM
{
  class Session;
}

class RtmEngine : public Plasma::DataEngine
{
Q_OBJECT

public:
  RtmEngine(QObject* parent, const QVariantList& args);
  virtual ~RtmEngine();
  
  bool authenticated() const;

  void updateTaskSource(const QString &taskid);
  void updateListSource(const QString &listid);
  
protected slots:
  void dataUpdate(const QString &source, const Plasma::DataEngine::Data &data);
  bool tokenCheck(bool success);

protected:
  void setupSource(Plasma::DataContainer* source);
  bool sourceRequestEvent(const QString& name);
  bool updateSourceEvent(const QString& source);

  Plasma::Service* serviceForSource(const QString& source);

  static const QString apiKey;
  static const QString sharedSecret;

  RTM::Session *session;
    
};

#endif // RTMENGINE_H
