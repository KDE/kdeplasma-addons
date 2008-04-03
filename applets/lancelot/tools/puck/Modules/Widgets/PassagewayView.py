from .. import WidgetHandlerManager
from .  import Panel
from .. import debug

import xml

class PassagewayView(Panel.PanelHandler):
    def name(self):
        return "Lancelot::PassagewayView"

    def include(self):
        return '#include <KIcon>\n#include <lancelot/widgets/PassagewayView.h>'

    def setup(self):
        setup = Panel.PanelHandler.setup(self)
        if self.hasAttribute('entranceTitle'):
            setup += self.attribute('name') + '->setEntranceTitle(i18n("' + self.attribute('entranceTitle') + '"));'
        if self.hasAttribute('entranceIcon'):
            setup += self.attribute('name') + '->setEntranceIcon(new KIcon("' + self.attribute('entranceIcon') + '"));'
        if self.hasAttribute('atlasTitle'):
            setup += self.attribute('name') + '->setAtlasTitle(i18n("' + self.attribute('atlasTitle') + '"));'
        if self.hasAttribute('atlasIcon'):
            setup += self.attribute('name') + '->setAtlasIcon(new KIcon("' + self.attribute('atlasIcon') + '"));'
        return setup

    def _construction(self):
        create = '${NAME} = new ' + self.name() + '("${NAME}")'
        create = create.replace('${NAME}', self.attribute('name'));
        return create

WidgetHandlerManager.addHandler(PassagewayView())
