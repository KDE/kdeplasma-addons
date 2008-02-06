#include "LancelotAppletConfig.h"

LancelotAppletConfig::LancelotAppletConfig()
{
    setCaption(i18n("Configure Lancelot Launcher"));

    QWidget * widget = new QWidget();
    m_ui.setupUi(widget);
    
    m_ui.iconLancelot->setPixmap((new KIcon("lancelot"))->pixmap(48));
    m_ui.iconKDE->setPixmap((new KIcon("kde"))->pixmap(48));
    m_ui.iconStartHere->setPixmap((new KIcon("start-here"))->pixmap(48));
    
    qbgIcons.addButton(m_ui.radioShowCategories);
    qbgIcons.addButton(m_ui.radioShowMenuIconOnly);
    
    qbgChooseIcon.addButton(m_ui.radioIconLancelot);
    qbgChooseIcon.addButton(m_ui.radioIconKDE);
    qbgChooseIcon.addButton(m_ui.radioIconStartHere);
    qbgChooseIcon.addButton(m_ui.radioIconCustom);
    
    qbgMenuActivation.addButton(m_ui.radioActivationHover);
    qbgMenuActivation.addButton(m_ui.radioActivationClick);
    
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

void LancelotAppletConfig::setShowCategories(bool value)
{
    if (value) {
        m_ui.radioShowCategories->click();
    } else {
        m_ui.radioShowMenuIconOnly->click();
    }
}

void LancelotAppletConfig::setClickActivation(bool value)
{
    if (value) {
        m_ui.radioActivationClick->click();
    } else {
        m_ui.radioActivationHover->click();
    }
}

void LancelotAppletConfig::setIcon(QString icon)
{
    if (icon == "lancelot") {
        m_ui.radioIconLancelot->click();
    } else if (icon == "kde") {
        m_ui.radioIconKDE->click();
    } else if (icon == "start-here") {
        m_ui.radioIconStartHere->click();
    } else {
        m_ui.radioIconCustom->click();
        m_ui.iconCustom->setIcon(icon);
    }
}
