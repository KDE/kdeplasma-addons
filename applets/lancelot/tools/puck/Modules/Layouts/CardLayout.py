from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class CardLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return 'Lancelot::CardLayout'

    def include(self):
        return '#include <lancelot/layouts/CardLayout.h>'

    def setup(self):
        setup = AbstractLayout.AbstractLayoutHandler.setup(self)

        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == 'item':
                continue

            line = self.attribute('name') + '->addItem(${ITEM_NAME}${CARD_NAME});';
            if child.hasAttribute(self.namespace() + ':cardName'):
                setup += line \
                    .replace('${ITEM_NAME}', child.getAttribute('name')) \
                    .replace('${CARD_NAME}', ((', "' + child.getAttribute(self.namespace() + ':cardName') + '"') \
                                               if child.hasAttribute(self.namespace() + ':cardName') else ''))
        return setup

LayoutHandlerManager.addHandler(CardLayoutHandler())
