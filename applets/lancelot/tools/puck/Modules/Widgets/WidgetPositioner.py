from .. import WidgetHandlerManager
from .  import AbstractWidget

import xml

class WidgetPositionerHandler(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::WidgetPositioner"

    def include(self):
        return '#include <lancelot/widgets/WidgetPositioner.h>'

    def _construction(self):
        return self.attribute("name") + " = new " + self.attribute("type") + \
            "(" + self.attribute('widgetName') + ", " + self.attribute('view') + ", " + self.attribute('parent') + ")"

WidgetHandlerManager.addHandler(WidgetPositionerHandler())
