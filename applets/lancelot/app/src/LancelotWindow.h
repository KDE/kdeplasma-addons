#ifndef LANCELOTWINDOW_H_
#define LANCELOTWINDOW_H_

#include <plasma/svg.h>
#include <plasma/plasma.h>
#include <plasma/corona.h>

#include "ui_LancelotWindowBase.h"

class LancelotWindow: public QFrame, public Ui::LancelotWindowBase
{
public:
	LancelotWindow();
	virtual ~LancelotWindow();
};

#endif /*LANCELOTWINDOW_H_*/
