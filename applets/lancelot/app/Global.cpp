#include "Global.h"

namespace Lancelot
{

bool Global::processGeometryChanges = false;
bool Global::processUpdateRequests = false;
QColor Global::textColorNormal = QColor(255, 255, 255);
QColor Global::textColorActive = QColor(0, 0, 0);

void Global::activateAll() {
    processGeometryChanges = true;
    processUpdateRequests = true;
}

void Global::deactivateAll() {
    processGeometryChanges = false;
    processUpdateRequests = false;
}



Global::Global()
{
}

Global::~Global()
{
}

}
