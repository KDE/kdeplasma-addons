from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class ExtenderButtonHanlder(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::ExtenderButton"
    
    def include(self):
        return "#include \"ExtenderButton.h\""

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        return setup
    
    def initialization(self):
        create = '${NAME} = new Lancelot::ExtenderButton("${NAME}", ${ICON}, ${TITLE}, ${DESCRIPTION});'
        create = create.replace('${NAME}', self.node().getAttribute('name'));
        
        if self.node().hasAttribute('ExtenderButton:icon'):
            create = create.replace('${ICON}', 'new KIcon("' + self.node().getAttribute('ExtenderButton:icon') + '")')
        else:
            create = create.replace('${ICON}', '0')
            
        if self.node().hasAttribute('ExtenderButton:title'):
            create = create.replace('${TITLE}', 'i18n("' + self.node().getAttribute('ExtenderButton:title') + '")')
        else:
            create = create.replace('${TITLE}', '""')
            
        if self.node().hasAttribute('ExtenderButton:description'):
            create = create.replace('${DESCRIPTION}', 'i18n("' + self.node().getAttribute('ExtenderButton:description') + '")')
        else:
            create = create.replace('${DESCRIPTION}', '""')
        
        return create

WidgetHandlerManager.addHandler(ExtenderButtonHanlder())
