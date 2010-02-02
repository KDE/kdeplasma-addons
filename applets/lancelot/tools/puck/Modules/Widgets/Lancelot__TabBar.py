from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsWidget

class Lancelot__TabBarHandler(QGraphicsWidget.QGraphicsWidgetHandler):
    def name(self):
        return "Lancelot::TabBar"

    def include(self):
        includes = "lancelot/widgets/TabBar.h QIcon QSize QString".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = QGraphicsWidget.QGraphicsWidgetHandler.setup(self)

        
        if self.hasAttribute('orientation'):
            setup += self.attribute('name') \
                  + '->setOrientation(Qt::' + self.attribute('orientation') + ');'
        
        if self.hasAttribute('textDirection'):
            setup += self.attribute('name') \
                  + '->setTextDirection(Qt::' + self.attribute('textDirection') + ');'
        
        if self.hasAttribute('currentTab'):
            setup += self.attribute('name') \
                  + '->setCurrentTab("' + self.attribute('currentTab') + '");'
        
        if self.hasAttribute('tabIconSize'):
            setup += self.attribute('name') \
                  + '->setTabIconSize(QSize(' + self.attribute('tabIconSize') + '));'
        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__TabBarHandler())


