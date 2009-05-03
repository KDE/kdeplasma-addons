#include "weatheri18ncatalog.h"
#include <QMutex>
#include <kglobal.h>
#include <klocale.h>

static bool catalogLoaded = false;
static QMutex loadingMutex;

void Weatheri18nCatalog::loadCatalog() {

	loadingMutex.lock();
	if (!catalogLoaded) {
		KGlobal::locale()->insertCatalog("libplasmaweather");
		catalogLoaded = true;
	}
	loadingMutex.unlock();
};
