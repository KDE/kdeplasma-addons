/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/** @file
* @brief This file contains the Qalculate! applet.
* @author Matteo Agostinelli <agostinelli@gmail.com> */

/*! \mainpage Qalculate! Plasma Applet
*
* \section intro_sec Introduction
*
* This is a calculator applet based on the Qalculate! library. It supports all basic operations
* plus some advanced features, such as symbolic expressions, unit and currency conversion, ...
*
* \section install_sec Installation
*
* Usual CMake / KDE procedure ...
*
*/

#ifndef QALCULATEAPPLET_H
#define QALCULATEAPPLET_H

#include <Plasma/PopupApplet>

namespace Plasma
{
class LineEdit;
class PushButton;
}
class OutputLabel;

class QalculateGraphicsWidget;
class QGraphicsLinearLayout;
class QalculateSettings;
class QalculateEngine;
class QalculateHistory;

class KConfigDialog;

//! Qalculate! applet
/*! This is the main class of the applet.
*/
class QalculateApplet : public Plasma::PopupApplet
{
    Q_OBJECT

public:
    //! Basic Create.
    QalculateApplet(QObject *parent, const QVariantList &args);
    //! Basic Destroy.
    ~QalculateApplet();

    //! Initializes the applet.
    void init();

    //! Returns the widget with the contents of the applet.
    virtual QGraphicsWidget* graphicsWidget();

public slots:
    //! The configuration has changed -> the plasmoid must react accordingly
    void configChanged();

    //! Navigate to the next history item
    void nextHistory();
    //! Navigate to the previous history item
    void previousHistory();

protected:
    void createConfigurationInterface(KConfigDialog *p_parent);

protected slots:
    //! Evaluate the expression entered by the user
    void evaluate();
    //! Evaluate the expression without adding it to the history
    void evalNoHist();
    //! Display the result of the calculation
    void displayResult(const QString&);
    //! A slot to process the received result from the engine
    void receivedResult(const QString&);
    //! Give focus to the input line
    void giveFocus();
    //! Show the history
    void showHistory();
    //! Hide the history
    void hideHistory();

private:
    //! The graphics widget
    QalculateGraphicsWidget* m_graphicsWidget;
    //! The main layout of the widget
    QGraphicsLinearLayout *m_layout;
    //! The applet's settings
    QalculateSettings* m_settings;
    //! The Qalculate! engine
    QalculateEngine* m_engine;
    //! History manager
    QalculateHistory* m_history;

    //! The input line where the user types the expression
    Plasma::LineEdit *m_input;
    //! The output label where the result is shown
    OutputLabel *m_output;
    //! Button which activates the menu
    Plasma::PushButton *m_historyButton;

    QGraphicsLinearLayout *m_historyList;

    //! The size of the result
    static int resultSize();
    static int simplificationSize();

    /** Gets a list of actions for the context menu. */
    virtual QList<QAction*> contextualActions();

    //! Setup actions
    void setupActions();

private slots:
    //! Create tooltip
    void createTooltip();
    //! Clear output label
    void clearOutputLabel();
};

#endif
