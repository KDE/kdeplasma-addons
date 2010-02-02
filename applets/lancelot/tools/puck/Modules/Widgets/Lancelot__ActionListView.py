from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__CustomListView

class Lancelot__ActionListViewHandler(Lancelot__CustomListView.Lancelot__CustomListViewHandler):
    def name(self):
        return "Lancelot::ActionListView"

    def include(self):
        includes = "lancelot/widgets/ActionListView.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__CustomListView.Lancelot__CustomListViewHandler.setup(self)


        if self.hasAttribute('extenderPosition'):
            setup += self.attribute('name') \
                  + '->setExtenderPosition(' + self.attribute('extenderPosition') + ');'

        if self.hasAttribute('categoriesActivable'):
            setup += self.attribute('name') \
                  + '->setCategoriesActivable(' + self.attribute('categoriesActivable') + ');'

        if self.hasAttribute('showsExtendersOutside'):
            setup += self.attribute('name') \
                  + '->setShowsExtendersOutside(' + self.attribute('showsExtendersOutside') + ');'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__ActionListViewHandler())


