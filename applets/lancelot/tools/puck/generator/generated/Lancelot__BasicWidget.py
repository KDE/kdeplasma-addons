from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Widget

class Lancelot__BasicWidgetHandler(Widget.WidgetHandler):
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
        setup = Widget.WidgetHandler.setup(self)

        
        if self.hasAttribute('icon'):
            setup += self.attribute('name') \
                  + '->setIcon(KIcon("' + self.attribute(icon) + '"));'
        
        if self.hasAttribute('iconSize'):
            setup += self.attribute('name') \
                  + '->setIconSize(QSize(' + self.attribute(iconSize) + '));'
        
        if self.hasAttribute('title'):
            setup += self.attribute('name') \
                  + '->setTitle("' + self.attribute(title) + '");'
        
        if self.hasAttribute('description'):
            setup += self.attribute('name') \
                  + '->setDescription("' + self.attribute(description) + '");'
        
        if self.hasAttribute('innerOrientation'):
            setup += self.attribute('name') \
                  + '->setInnerOrientation(Qt::' + self.attribute(innerOrientation) + ');'
        

        return setup;


