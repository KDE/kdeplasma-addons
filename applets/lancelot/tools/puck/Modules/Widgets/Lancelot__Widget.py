from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsWidget

class Lancelot__WidgetHandler(QGraphicsWidget.QGraphicsWidgetHandler):
    def name(self):
        return "Lancelot::Widget"

    def include(self):
        includes = "lancelot/widgets/Widget.h QString".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = QGraphicsWidget.QGraphicsWidgetHandler.setup(self)


        if self.hasAttribute('group'):
            setup += self.attribute('name') \
                  + '->setGroupByName("' + self.attribute('group') + '");'

        if self.hasAttribute('down'):
            setup += self.attribute('name') \
                  + '->setDown(' + self.attribute('down') + ');'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__WidgetHandler())


