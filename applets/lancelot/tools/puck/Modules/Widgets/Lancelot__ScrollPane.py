from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__ScrollPaneHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::ScrollPane"

    def include(self):
        includes = "lancelot/widgets/ScrollPane.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)


        if self.hasAttribute('scrollPosition'):
            setup += self.attribute('name') \
                  + '->setScrollPosition(QPointF(' + self.attribute('scrollPosition') + '));'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__ScrollPaneHandler())


