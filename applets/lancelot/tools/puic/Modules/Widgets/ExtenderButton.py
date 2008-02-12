from .. import WidgetHandlerManager
from .  import AbstractWidget
from .. import debug

import xml

class ExtenderButtonHandler(AbstractWidget.AbstractWidgetHandler):
    def name(self):
        return "Lancelot::ExtenderButton"
    
    def include(self):
        return '#include <KIcon>\n#include "ExtenderButton.h"'

    def setup(self):
        setup = AbstractWidget.AbstractWidgetHandler.setup(self)
        if self.hasAttribute('group'):
            setup += self.attribute('name') + '->setGroupByName("' + self.attribute('group') + '");'
        if self.hasAttribute('activation'):
            setup += self.attribute('name') + '->setActivationMethod(Lancelot::ExtenderButton::' \
                  + self.attribute('activation') + ');'
        if self.hasAttribute('extenderPosition'):
            setup += self.attribute('name') + '->setExtenderPosition(Lancelot::ExtenderButton::' \
                  + self.attribute('extenderPosition') + ');'
        if self.hasAttribute('iconSize'):
            setup += self.attribute('name') + '->setIconSize(QSize(' \
                  + self.attribute('iconSize') + '));'
        if self.hasAttribute('orientation'):
            setup += self.attribute('name') + '->setInnerOrientation(Lancelot::BaseActionWidget::' \
                  + self.attribute('orientation') + ');'
        return setup
    
    def _construction(self):
        create = '${NAME} = new ' + self.name() + '("${NAME}", ${ICON}, ${TITLE}, ${DESCRIPTION})'
        create = create.replace('${NAME}', self.attribute('name'));
        
        if self.hasAttribute('icon'):
            create = create.replace('${ICON}', 'new KIcon("' + self.attribute('icon') + '")')
        else:
            create = create.replace('${ICON}', '0')
            
        if self.hasAttribute('title'):
            create = create.replace('${TITLE}', 'i18n("' + self.attribute('title') + '")')
        else:
            create = create.replace('${TITLE}', '""')
            
        if self.hasAttribute('description'):
            create = create.replace('${DESCRIPTION}', 'i18n("' + self.attribute('description') + '")')
        else:
            create = create.replace('${DESCRIPTION}', '""')
        
        return create

WidgetHandlerManager.addHandler(ExtenderButtonHandler())
