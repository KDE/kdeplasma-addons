from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class ExtenderButtonHanlder(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::ExtenderButton"
    
    def include(self):
        return '#include <KIcon>\n#include "ExtenderButton.h"'

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.node().hasAttribute('ExtenderButton:group'):
            setup += self.node().getAttribute('name') + '->setGroupByName("' + self.node().getAttribute('ExtenderButton:group') + '");'
        if self.node().hasAttribute('ExtenderButton:activation'):
            setup += self.node().getAttribute('name') + '->setActivationMethod(Lancelot::ExtenderButton::' \
                  + self.node().getAttribute('ExtenderButton:activation') + ');'
        if self.node().hasAttribute('ExtenderButton:extenderPosition'):
            setup += self.node().getAttribute('name') + '->setExtenderPosition(Lancelot::ExtenderButton::' \
                  + self.node().getAttribute('ExtenderButton:extenderPosition') + ');'
        if self.node().hasAttribute('ExtenderButton:iconSize'):
            setup += self.node().getAttribute('name') + '->setIconSize(QSize(' \
                  + self.node().getAttribute('ExtenderButton:iconSize') + '));'
        if self.node().hasAttribute('ExtenderButton:orientation'):
            setup += self.node().getAttribute('name') + '->setInnerOrientation(Lancelot::BaseActionWidget::' \
                  + self.node().getAttribute('ExtenderButton:orientation') + ');'
        return setup
    
    def _construction(self):
        create = '${NAME} = new Lancelot::ExtenderButton("${NAME}", ${ICON}, ${TITLE}, ${DESCRIPTION})'
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
