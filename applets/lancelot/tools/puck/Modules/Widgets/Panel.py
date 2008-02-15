from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class PanelHandler(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::Panel"
    
    def include(self):
        return '#include <KIcon>\n#include "Panel.h"'

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.hasAttribute('group'):
            setup += self.attribute('name') + '->setGroupByName("' + self.attribute('group') + '");'
        if self.hasAttribute('background'):
            setup += self.attribute('name') + '->setBackground("' + self.attribute('background') + '");'
        if self.hasAttribute('iconSize'):
            setup += self.attribute('name') + '->setIconSize(QSize(' \
                  + self.attribute('iconSize') + '));'
                  
        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue
            
            if child.localName == "widget":
                setup += self.attribute("name") + "->setWidget(" + child.getAttribute("name") + ");\n"
            elif child.localName == "layout":
                setup += self.attribute("name") + "->setLayout(" + child.getAttribute("name") + ");\n"
        return setup
    
    def _construction(self):
        create = '${NAME} = new Lancelot::Panel("${NAME}", ${ICON}, ${TITLE})'
        create = create.replace('${NAME}', self.attribute('name'));
        
        if self.hasAttribute('icon'):
            create = create.replace('${ICON}', 'new KIcon("' + self.attribute('icon') + '")')
        else:
            create = create.replace('${ICON}', '0')
            
        if self.hasAttribute('title'):
            create = create.replace('${TITLE}', 'i18n("' + self.attribute('title') + '")')
        else:
            create = create.replace('${TITLE}', '""')
        
        return create

WidgetHandlerManager.addHandler(PanelHandler())
