from .. import WidgetHandlerManager
from .  import ExtenderButton
from .. import debug

import xml

class ToggleExtenderButtonHandler(ExtenderButton.ExtenderButtonHandler):
    def name(self):
        return "Lancelot::ToggleExtenderButton"
    
    def include(self):
        return '#include <KIcon>\n#include "ToggleExtenderButton.h"'

WidgetHandlerManager.addHandler(ToggleExtenderButtonHandler())
