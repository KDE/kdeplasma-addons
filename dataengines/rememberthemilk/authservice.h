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

#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>
#include <QTimer>

namespace RTM {
  class Session;
}

class TaskSource;

class AuthService : public Plasma::Service
{
public:
  AuthService(RTM::Session *session, QObject* parent);
protected:
  virtual Plasma::ServiceJob* createJob(const QString& operation, QMap< QString, QVariant >& parameters);
  TaskSource* m_source;
  RTM::Session* m_session;
};

class AuthJob : public Plasma::ServiceJob
{
  Q_OBJECT
public:
    AuthJob(RTM::Session* session, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent);
    
public slots:
    void start();

signals:
    void authUrlReady(QString url);

private slots:
    void tokenReply(bool tokenValid);
    
protected:
    RTM::Session* m_session;
    int autoRetry;
};

#endif // AUTHSERVICE_H
