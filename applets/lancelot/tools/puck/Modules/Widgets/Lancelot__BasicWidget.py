from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__BasicWidgetHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::BasicWidget"

    def include(self):
        includes = "lancelot/widgets/BasicWidget.h QIcon QSize QString".split(" ")
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
                  + '->setTitle(i18n("' + self.attribute('title') + '"));'
        
        if self.hasAttribute('description'):
            setup += self.attribute('name') \
                  + '->setDescription(i18n("' + self.attribute('description') + '"));'
        
        if self.hasAttribute('innerOrientation'):
            setup += self.attribute('name') \
                  + '->setInnerOrientation(Qt::' + self.attribute('innerOrientation') + ');'
        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__BasicWidgetHandler())


