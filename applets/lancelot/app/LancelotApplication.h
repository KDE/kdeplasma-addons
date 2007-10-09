#ifndef LANCELOTAPPLICATION_H_
#define LANCELOTAPPLICATION_H_

#include <kuniqueapplication.h>
#include <QTimer>
#include <QSet>

class LancelotWindow;

class LancelotApplication: public KUniqueApplication
{
    Q_OBJECT
    //CLASSINFO("D-Bus Interface", "org.kde.lancelot")
public:
	LancelotApplication(int argc, char **argv);
	virtual ~LancelotApplication();
	
	static int main(int argc, char **argv);
	
public Q_SLOTS:
    bool show();
    bool hide();
    bool showItem(QString name);
    
    int addClient();
    bool removeClient(int id);
       
protected:
    static LancelotApplication * application;
    LancelotWindow * window;
    
private:
    int m_clientsNumber;
    int m_lastID;
    QSet<int> m_clients;

};

#endif /*LANCELOTAPPLICATION_H_*/
