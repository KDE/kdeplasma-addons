
from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class LancelotWidgetHandler(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::Widget"
    
    def include(self):
        return '#include "Widget.h"'

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.hasAttribute('group'):
            setup += self.attribute('name') + '->setGroupByName("' + self.attribute('group') + '");'
        return setup
    
    def _construction(self):
        create = '${NAME} = new ' + self.name() + '("${NAME}")'
        create = create.replace('${NAME}', self.attribute('name'));
        return create

WidgetHandlerManager.addHandler(LancelotWidgetHandler())
