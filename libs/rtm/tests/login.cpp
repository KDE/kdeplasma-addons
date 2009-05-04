#include "session.h"
#include <QString>
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

#include <KIO/Job>
#include <KIO/NetAccess>

#include <KHTMLPart>
#include <KHTMLView>
#include <DOM/HTMLInputElement>
#include <DOM/HTMLDocument>

int main(int argc, char* argv[]) {
  KAboutData about("login", 0, ki18n("RTM Login Test"), "1.0", ki18n("A Simple RTM Login Test"),
                  KAboutData::License_GPL, ki18n("(C) 2009 Andrew Stromme"), KLocalizedString(), 0, "astromme@chatonka.com");
  KCmdLineArgs::init(argc, argv, &about);
  KApplication app;

  RTM::Session *session = new RTM::Session("myapikey", "mysharedsecret", RTM::Delete, QString(), &app);
  
  session->login("myuser", "mypass");
  
  app.exec();
}
