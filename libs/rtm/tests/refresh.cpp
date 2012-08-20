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

#include <QString>
#include <QTimer>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <QtDebug>

#include <KIO/Job>
#include <KIO/NetAccess>

#include <KHTMLPart>
#include <KHTMLView>
#include <DOM/HTMLInputElement>
#include <DOM/HTMLDocument>


class Refresher : public QObject
{
  Q_OBJECT
  
  public:
    Refresher() {}
    ~Refresher() {}
    
    RTM::Session *session;
    
  public slots:
    void refresh() {
      qDebug() << "Refreshing";
      session->refreshListsFromServer();
      session->refreshTasksFromServer();
    }
};

int main(int argc, char* argv[]) {
  KAboutData about("login", 0, ki18n("RTM Refresh Test"), "1.0", ki18n("A Simple RTM Refresh Test"),
                    KAboutData::License_GPL, ki18n("(C) 2009 Andrew Stromme"), KLocalizedString(), 0, "astromme@chatonka.com");
                    
  KCmdLineArgs::init(argc, argv, &about);
  KApplication app;
                    
  RTM::Session *session = new RTM::Session("myapikey", "mysharedsecret", RTM::Delete, QString(), &app);
  
  session->showLoginWindow();
  
  Refresher refresher;
  refresher.session = session;
  
  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), &refresher, SLOT(refresh()));
  timer.start(3000);
  
  QTimer quit;
  QObject::connect(&quit, SIGNAL(timeout()), &app, SLOT(quit()));
  timer.start(4000);
  
  app.exec();
}

#include "refresh.moc"
