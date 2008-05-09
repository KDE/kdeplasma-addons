from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__PanelHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::Panel"

    def include(self):
        includes = "lancelot/widgets/Panel.h QIcon QSize QString".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)

        
        if self.hasAttribute('icon'):
            setup += self.attribute('name') \
                  + '->setIcon(KIcon("' + self.attribute('icon') + '"));'
        
        if self.hasAttribute('iconSize'):
            setup += self.attribute('name') \
                  + '->setIconSize(QSize(' + self.attribute('iconSize') + '));'
        
        if self.hasAttribute('title'):
            setup += self.attribute('name') \
                  + '->setTitle("' + self.attribute('title') + '");'
        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__PanelHandler())


