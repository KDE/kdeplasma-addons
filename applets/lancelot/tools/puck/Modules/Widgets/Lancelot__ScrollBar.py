from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__ScrollBarHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::ScrollBar"

    def include(self):
        includes = "lancelot/widgets/ScrollBar.h lancelot/lancelot.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)

        
        if self.hasAttribute('minimum'):
            setup += self.attribute('name') \
                  + '->setMinimum(' + self.attribute('minimum') + ');'
        
        if self.hasAttribute('maximum'):
            setup += self.attribute('name') \
                  + '->setMaximum(' + self.attribute('maximum') + ');'
        
        if self.hasAttribute('value'):
            setup += self.attribute('name') \
                  + '->setValue(' + self.attribute('value') + ');'
        
        if self.hasAttribute('viewSize'):
            setup += self.attribute('name') \
                  + '->setViewSize(' + self.attribute('viewSize') + ');'
        
        if self.hasAttribute('pageSize'):
            setup += self.attribute('name') \
                  + '->setPageSize(' + self.attribute('pageSize') + ');'
        
        if self.hasAttribute('stepSize'):
            setup += self.attribute('name') \
                  + '->setStepSize(' + self.attribute('stepSize') + ');'
        
        if self.hasAttribute('orientation'):
            setup += self.attribute('name') \
                  + '->setOrientation(Qt::' + self.attribute('orientation') + ');'
        
        if self.hasAttribute('activationMethod'):
            setup += self.attribute('name') \
                  + '->setActivationMethod(Lancelot::' + self.attribute('activationMethod') + 'Activate);'
        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__ScrollBarHandler())


