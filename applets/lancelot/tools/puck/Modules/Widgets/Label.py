from .. import WidgetHandlerManager
from .  import AbstractWidget

import xml

class LabelHandler(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Plasma::Label"
    
    def include(self):
        return "#include <plasma/widgets/label.h>"

    def _construction(self):
        return self.attribute("name") + " = new " + self.attribute("type") + "(0)"

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.hasAttribute('text'):
            setup += self.attribute('name') + '->setText("' + self.attribute('text') + '");'
        return setup

WidgetHandlerManager.addHandler(LabelHandler())
