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

#include "session.h"

#include "loginwidget.cpp"

#include <QString>
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <QtDebug>

int main(int argc, char* argv[]) { 
  KAboutData about("login", 0, ki18n("RTM Login Test"), "1.0", ki18n("A Simple RTM Login Test"),
                  KAboutData::License_GPL, ki18n("(C) 2009 Andrew Stromme"), KLocalizedString(), 0, "astromme@chatonka.com");
  KCmdLineArgs::init(argc, argv, &about);
  
  KApplication app;
  
  RTM::Session *session = new RTM::Session("myapikey", "mysharedsecret", RTM::Delete, QString(), &app);
  
  LoginWidget *login = new LoginWidget();
  login->setWebUrl(session->getAuthUrl());
  login->show();
  
  return app.exec();
}

