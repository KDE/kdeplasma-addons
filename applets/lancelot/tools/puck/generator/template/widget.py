from .. import WidgetHandlerManager
from .. import AbstractItem
${PYTHON_IMPORT}

class ${PYTHON_NAME}Handler(${PARENT_PYTHON_NAME}.${PARENT_PYTHON_NAME}Handler):
    def name(self):
        return "${NAME}"

    def include(self):
        includes = "${INCLUDES}".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = ${PARENT_PYTHON_NAME}.${PARENT_PYTHON_NAME}Handler.setup(self)

        ${PROPERTY_SETUP_BEGIN}
        if self.hasAttribute('${PROPERTY_NAME}'):
            setup += self.attribute('name') \
                  + '->${PROPERTY_SETTER}(${PROPERTY_VALUE_PREFIX}' + self.attribute('${PROPERTY_NAME}') + '${PROPERTY_VALUE_SUFIX});'
        ${PROPERTY_SETUP_END}

        return setup;

WidgetHandlerManager.addHandler(${PYTHON_NAME}Handler())

