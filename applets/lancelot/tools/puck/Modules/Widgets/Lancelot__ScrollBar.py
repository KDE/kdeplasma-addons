from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Plasma__ScrollBar

class Lancelot__ScrollBarHandler(Plasma__ScrollBar.Plasma__ScrollBarHandler):
    def name(self):
        return "Lancelot::ScrollBar"

    def include(self):
        includes = "lancelot/widgets/ScrollBar.h lancelot/lancelot.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Plasma__ScrollBar.Plasma__ScrollBarHandler.setup(self)


        if self.hasAttribute('activationMethod'):
            setup += self.attribute('name') \
                  + '->setActivationMethod(' + self.attribute('activationMethod') + ');'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__ScrollBarHandler())


