from .. import WidgetHandlerManager
from .. import AbstractItem


class QGraphicsLayoutItemHandler(AbstractItem.AbstractItemHandler):
    def name(self):
        return "QGraphicsLayoutItem"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = AbstractItem.AbstractItemHandler.setup(self)


        if self.hasAttribute('minimumSize'):
            setup += self.attribute('name') \
                  + '->setMinimumSize(QSize(' + self.attribute('minimumSize') + '));'

        if self.hasAttribute('minimumWidth'):
            setup += self.attribute('name') \
                  + '->setMinimumWidth(' + self.attribute('minimumWidth') + ');'

        if self.hasAttribute('minimumHeight'):
            setup += self.attribute('name') \
                  + '->setMinimumHeight(' + self.attribute('minimumHeight') + ');'

        if self.hasAttribute('maximumSize'):
            setup += self.attribute('name') \
                  + '->setMaximumSize(QSize(' + self.attribute('maximumSize') + '));'

        if self.hasAttribute('maximumWidth'):
            setup += self.attribute('name') \
                  + '->setMaximumWidth(' + self.attribute('maximumWidth') + ');'

        if self.hasAttribute('maximumHeight'):
            setup += self.attribute('name') \
                  + '->setMaximumHeight(' + self.attribute('maximumHeight') + ');'

        if self.hasAttribute('preferredSize'):
            setup += self.attribute('name') \
                  + '->setPreferredSize(QSize(' + self.attribute('preferredSize') + '));'

        if self.hasAttribute('preferredWidth'):
            setup += self.attribute('name') \
                  + '->setPreferredWidth(' + self.attribute('preferredWidth') + ');'

        if self.hasAttribute('preferredHeight'):
            setup += self.attribute('name') \
                  + '->setPreferredHeight(' + self.attribute('preferredHeight') + ');'


        return setup;

WidgetHandlerManager.addHandler(QGraphicsLayoutItemHandler())


