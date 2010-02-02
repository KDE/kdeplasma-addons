from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Plasma__IconWidget

class Lancelot__HoverIconHandler(Plasma__IconWidget.Plasma__IconWidgetHandler):
    def name(self):
        return "Lancelot::HoverIcon"

    def include(self):
        includes = "lancelot/widgets/HoverIcon.h plasma/widgets/icon.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Plasma__IconWidget.Plasma__IconWidgetHandler.setup(self)


        if self.hasAttribute('activationMethod'):
            setup += self.attribute('name') \
                  + '->setActivationMethod(' + self.attribute('activationMethod') + ');'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__HoverIconHandler())


