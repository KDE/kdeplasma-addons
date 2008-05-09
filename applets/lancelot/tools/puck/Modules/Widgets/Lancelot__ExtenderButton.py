from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__BasicWidget

class Lancelot__ExtenderButtonHandler(Lancelot__BasicWidget.Lancelot__BasicWidgetHandler):
    def name(self):
        return "Lancelot::ExtenderButton"

    def include(self):
        includes = "lancelot/widgets/ExtenderButton.h lancelot/lancelot.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__BasicWidget.Lancelot__BasicWidgetHandler.setup(self)

        
        if self.hasAttribute('extenderPosition'):
            setup += self.attribute('name') \
                  + '->setExtenderPosition(Lancelot::' + self.attribute('extenderPosition') + 'Extender);'
        
        if self.hasAttribute('activationMethod'):
            setup += self.attribute('name') \
                  + '->setActivationMethod(Lancelot::' + self.attribute('activationMethod') + 'Activate);'
        
        if self.hasAttribute('checkable'):
            setup += self.attribute('name') \
                  + '->setCheckable(' + self.attribute('checkable') + ');'
        
        if self.hasAttribute('checked'):
            setup += self.attribute('name') \
                  + '->setChecked(' + self.attribute('checked') + ');'
        
        if self.hasAttribute('down'):
            setup += self.attribute('name') \
                  + '->setDown(' + self.attribute('down') + ');'
        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__ExtenderButtonHandler())


