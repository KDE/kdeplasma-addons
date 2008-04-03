from .. import WidgetHandlerManager
from .  import LancelotWidget
from .. import debug

import xml

class ActionListViewHandler(LancelotWidget.LancelotWidgetHandler):
    def name(self):
        return "Lancelot::ActionListView"

    def include(self):
        return '#include <KIcon>\n#include <lancelot/widgets/ActionListView.h>'

    def setup(self):
        setup = LancelotWidget.LancelotWidgetHandler.setup(self)
        if self.hasAttribute('itemsGroup'):
            setup += self.attribute('name') + '->setItemsGroupByName("' + self.attribute('itemsGroup') + '");'
        if self.hasAttribute('extenderPosition'):
            setup += self.attribute('name') + '->setExtenderPosition(Lancelot::ExtenderButton::' + self.attribute('extenderPosition') + ');'
        return setup

WidgetHandlerManager.addHandler(ActionListViewHandler())
