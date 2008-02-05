#ifndef LANCELOTAPPLETCONFIG_H_
#define LANCELOTAPPLETCONFIG_H_

#include <ui_LancelotAppletConfigBase.h>

class LancelotAppletConfig : public KDialog {
public:
	LancelotAppletConfig();
	virtual ~LancelotAppletConfig();
	
	bool showCategories() const;
	bool clickActivation() const;
	QString icon() const;
	
private:
    Ui::LancelotAppletConfigBase m_ui;
};

#endif /*LANCELOTAPPLETCONFIG_H_*/
