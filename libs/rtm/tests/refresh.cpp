
#include "session.h"

#include <QString>
#include <QTimer>

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


class Refresher : public QObject
{
  Q_OBJECT
  
  public:
    Refresher() {}
    ~Refresher() {}
    
    RTM::Session *session;
    
  public slots:
    void refresh() {
      kDebug() << "Refreshing";
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
