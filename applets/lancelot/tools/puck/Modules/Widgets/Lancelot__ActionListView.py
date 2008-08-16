from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__ActionListViewHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::ActionListView"

    def include(self):
        includes = "lancelot/widgets/ActionListView.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)

        if self.hasAttribute('itemsGroup'):
            setup += self.attribute('name') \
                  + '->setItemsGroupByName("' + self.attribute('itemsGroup') + '");'

        if self.hasAttribute('categoriesGroup'):
            setup += self.attribute('name') \
                  + '->setItemsGroupByName("' + self.attribute('categoriesGroup') + '");'

        if self.hasAttribute('extenderPosition'):
            setup += self.attribute('name') \
                  + '->setExtenderPosition(Lancelot::' + self.attribute('extenderPosition') + 'Extender);'

        return setup;

WidgetHandlerManager.addHandler(Lancelot__ActionListViewHandler())


