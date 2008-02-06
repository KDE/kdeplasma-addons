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
	
    void setShowCategories(bool value);
    void setClickActivation(bool value);
    void setIcon(QString icon);
    
private:
    Ui::LancelotAppletConfigBase m_ui;
    QButtonGroup qbgIcons;
    QButtonGroup qbgChooseIcon;
    QButtonGroup qbgMenuActivation;
};

#endif /*LANCELOTAPPLETCONFIG_H_*/
