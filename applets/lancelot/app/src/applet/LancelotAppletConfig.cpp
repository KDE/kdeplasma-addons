#include "LancelotAppletConfig.h"

LancelotAppletConfig::LancelotAppletConfig()
{
    setCaption(i18n("Configure Lancelot Launcher"));

    QWidget * widget = new QWidget();
    m_ui.setupUi(widget);
    
    m_ui.iconLancelot->setPixmap((new KIcon("lancelot"))->pixmap(48));
    m_ui.iconKDE->setPixmap((new KIcon("kde"))->pixmap(48));
    m_ui.iconStartHere->setPixmap((new KIcon("start-here"))->pixmap(48));
    
    setMainWidget(widget);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
}

LancelotAppletConfig::~LancelotAppletConfig()
{
}

bool LancelotAppletConfig::showCategories() const
{
    return m_ui.radioShowCategories->isChecked();
}

bool LancelotAppletConfig::clickActivation() const
{
    return m_ui.radioActivationClick->isChecked();
}

QString LancelotAppletConfig::icon() const
{
    if (m_ui.radioIconLancelot->isChecked()) {
        return "lancelot";
    }
    if (m_ui.radioIconKDE->isChecked()) {
        return "kde";
    }
    if (m_ui.radioIconStartHere->isChecked()) {
        return "start-here";
    }
    if (m_ui.radioIconCustom->isChecked()) {
        return m_ui.iconCustom->icon();
    }
}
