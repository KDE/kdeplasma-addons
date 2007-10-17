#ifndef LANCELOT_GLOBAL_H_
#define LANCELOT_GLOBAL_H_

#include <QColor>

namespace Lancelot
{

class Global
{
public:
	Global();
	virtual ~Global();
	
	static bool processGeometryChanges;
	static bool processUpdateRequests;
	
    static QColor textColorNormal;
    static QColor textColorActive;
	
	static void activateAll();
	static void deactivateAll();
};

}

#endif /*GLOBAL_H_*/
