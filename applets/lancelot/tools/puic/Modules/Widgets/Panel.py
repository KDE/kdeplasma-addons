from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class PanelHanlder(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::Panel"
    
    def include(self):
        return '#include <KIcon>\n#include "Panel.h"'

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.node().hasAttribute('Panel:group'):
            setup += self.node().getAttribute('name') + '->setGroupByName("' + self.node().getAttribute('ExtenderButton:group') + '");'
        if self.node().hasAttribute('Panel:iconSize'):
            setup += self.node().getAttribute('name') + '->setIconSize(QSize(' \
                  + self.node().getAttribute('ExtenderButton:iconSize') + '));'
                  
        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue
            
            if child.localName == "widget":
                setup += self.node().getAttribute("name") + "->setWidget(" + child.getAttribute("name") + ");\n"
            elif child.localName == "layout":
                setup += self.node().getAttribute("name") + "->setLayout(" + child.getAttribute("name") + ");\n"
        return setup
    
    def _construction(self):
        create = '${NAME} = new Lancelot::Panel("${NAME}", ${ICON}, ${TITLE})'
        create = create.replace('${NAME}', self.node().getAttribute('name'));
        
        if self.node().hasAttribute('Panel:icon'):
            create = create.replace('${ICON}', 'new KIcon("' + self.node().getAttribute('Panel:icon') + '")')
        else:
            create = create.replace('${ICON}', '0')
            
        if self.node().hasAttribute('Panel:title'):
            create = create.replace('${TITLE}', 'i18n("' + self.node().getAttribute('Panel:title') + '")')
        else:
            create = create.replace('${TITLE}', '""')
        
        return create

WidgetHandlerManager.addHandler(PanelHanlder())
