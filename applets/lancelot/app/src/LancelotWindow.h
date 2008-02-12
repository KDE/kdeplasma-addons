#ifndef LANCELOTWINDOW_H_
#define LANCELOTWINDOW_H_

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

#include "ui_LancelotWindowBase.h"

class CustomGraphicsView;

class LancelotWindow: public QWidget, public Ui::LancelotWindowBase
{
public:
    LancelotWindow();
    virtual ~LancelotWindow();

public Q_SLOTS:
    void lancelotShow(int x, int y);
    void lancelotShowItem(int x, int y, QString name);
    void lancelotHide(bool immediate = false);
    void search(const QString & string);

public:
    QStringList sectionIDs();
    QStringList sectionNames();
    QStringList sectionIcons();
    void focusOutEvent(QFocusEvent * event);
    
protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);
    void resizeWindow(QSize newSize);
    void showWindow(int x, int y);

private:
    Plasma::Widget      * m_root;
    CustomGraphicsView  * m_view;
    Plasma::Corona      * m_corona;
    QVBoxLayout         * m_layout;
    QTimer                m_hideTimer;
    bool                  m_hovered;

    Lancelot::Instance  * instance;
};

#endif /*LANCELOTWINDOW_H_*/
