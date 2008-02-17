from .. import AbstractItem
from .. import WidgetHandlerManager

class AbstractWidgetHandler(AbstractItem.AbstractItemHandler):

    def setup(self):
        setup = ''

        if self.hasAttribute('minimumSize'):
            setup += attribute('name') + '->setMinimumSize(' + self.attribute('minimumSize') + ');\n';
            
        if self.hasAttribute('maximumSize'):
            setup += attribute('name') + '->setMaximumSize(' + self.attribute('maximumSize') + ');\n';
            
        if self.hasAttribute('opacity'):
            setup += self.attribute('name') + '->setOpacity(' + self.attribute('opacity') + ');\n';
            
        return setup

    def initialization(self):
        return WidgetHandlerManager.root() + '->addChild(' + self._construction() + ');'

WidgetHandlerManager.addHandler(AbstractWidgetHandler())