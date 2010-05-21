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

#include "qalculate_applet.h"
#include "qalculate_settings.h"
#include "qalculate_engine.h"
#include "qalculate_history.h"
#include "qalculate_graphicswidget.h"

#include "outputlabel.h"

#include <KLocale>
#include <KLineEdit>
#include <KIcon>
#include <KAction>

#include <Plasma/LineEdit>
#include <Plasma/Label>
#include <Plasma/Theme>
#include <Plasma/Containment>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>

K_EXPORT_PLASMA_APPLET(qalculate, QalculateApplet)

QalculateApplet::QalculateApplet(QObject *parent, const QVariantList &args)
        : Plasma::PopupApplet(parent, args),
        m_graphicsWidget(0),
        m_input(0),
        m_output(0)
{
    // this will get us the standard applet background, for free!
    setBackgroundHints(DefaultBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    m_settings = new QalculateSettings(this);
    m_engine = new QalculateEngine(m_settings);
    m_history = new QalculateHistory(this);
    connect(m_engine, SIGNAL(formattedResultReady(const QString&)), this, SLOT(displayResult(const QString&)));
    connect(m_engine, SIGNAL(resultReady(const QString&)), this, SLOT(createTooltip()));
    connect(m_engine, SIGNAL(resultReady(const QString&)), this, SLOT(receivedResult(const QString&)));
    connect(m_settings, SIGNAL(configChanged()), this, SLOT(evalNoHist()));
  
    setHasConfigurationInterface(true);
}

QalculateApplet::~QalculateApplet()
{
    if (hasFailedToLaunch()) {

    } else {
        delete m_input;
        delete m_output;
        delete m_graphicsWidget;
    }
}

void QalculateApplet::init()
{
    if (m_settings->updateExchangeRatesAtStartup()) {
        m_engine->updateExchangeRates();
    }
    
    graphicsWidget();
    setupActions();
    setPopupIcon(KIcon("qalculate"));
    createTooltip();
}

void QalculateApplet::setupActions()
{
    KAction *actionCopy = new KAction(KIcon("edit-copy"), i18n("&Copy result to clipboard"), this);
    actionCopy->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(actionCopy, SIGNAL(triggered(bool)), m_engine, SLOT(copyToClipboard(bool)));
    addAction("copyToClipboard", actionCopy);
}

int QalculateApplet::simplificationSize()
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).pointSize();
}

int QalculateApplet::resultSize()
{
    return simplificationSize()*2;
}

QGraphicsWidget* QalculateApplet::graphicsWidget()
{
    if (!m_graphicsWidget) {
        m_graphicsWidget = new QalculateGraphicsWidget(this);
        m_graphicsWidget->setMinimumSize(200, 150);
        m_graphicsWidget->setPreferredSize(300, 200);

        m_input = new Plasma::LineEdit;
        m_input->nativeWidget()->setClickMessage(i18n("Enter an expression..."));
        m_input->nativeWidget()->setClearButtonShown(true);
        m_input->setAttribute(Qt::WA_NoSystemBackground);
        connect(m_input, SIGNAL(returnPressed()), this, SLOT(evaluate()));
        connect(m_input->nativeWidget(), SIGNAL(clearButtonClicked()), this, SLOT(clearOutputLabel()));
        connect(m_input->nativeWidget(), SIGNAL(editingFinished()), this, SLOT(evalNoHist()));
        m_input->setAcceptedMouseButtons(Qt::LeftButton);
        m_input->setFocusPolicy(Qt::StrongFocus);
           
        m_output = new OutputLabel;
        m_output->nativeWidget()->setAlignment(Qt::AlignCenter);
        QFont f = m_output->nativeWidget()->font();
        f.setBold(true);
        f.setPointSize(resultSize() / 2);
        m_output->nativeWidget()->setFont(f);
        m_output->setFocusPolicy(Qt::NoFocus);
        connect(m_output, SIGNAL(clicked()), this, SLOT(giveFocus()));

        QPalette palette = m_output->palette();
        palette.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        m_output->nativeWidget()->setPalette(palette);

        m_layout = new QGraphicsLinearLayout(Qt::Vertical);
        m_layout->addItem(m_input);
        m_layout->addItem(m_output);

        m_graphicsWidget->setLayout(m_layout);
        m_graphicsWidget->setFocusPolicy(Qt::StrongFocus);

        configChanged();
        clearOutputLabel();
        
        connect(m_graphicsWidget, SIGNAL(giveFocus()), this, SLOT(giveFocus()));
        connect(m_graphicsWidget, SIGNAL(nextHistory()), this, SLOT(nextHistory()));
        connect(m_graphicsWidget, SIGNAL(previousHistory()), this, SLOT(previousHistory()));
	//connect(this, SIGNAL(activated()), this, SLOT(giveFocus()));
    }

    return m_graphicsWidget;
}

QList< QAction* > QalculateApplet::contextualActions()
{
    return QList< QAction* >() << action("copyToClipboard");
}

void QalculateApplet::createTooltip()
{
    if (containment()->containmentType() == Plasma::Containment::DesktopContainment) {
        Plasma::ToolTipManager::self()->hide(this);
        return;
    }
    
    Plasma::ToolTipContent data;
    data.setMainText(i18n("Qalculate!"));
    data.setSubText(m_engine->lastResult());
    data.setImage(KIcon("qalculate").pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(this, data);
}

void QalculateApplet::createConfigurationInterface(KConfigDialog* parent)
{
    m_settings->createConfigurationInterface(parent);
}

void QalculateApplet::evaluate()
{
    evalNoHist();
    m_history->addItem(m_input->text());
}

void QalculateApplet::evalNoHist()
{
    if (m_input->text().isEmpty()) {
        clearOutputLabel();
        return;
    }
    
    m_engine->evaluate(m_input->text().replace(KGlobal::locale()->decimalSymbol(), "."));
}

void QalculateApplet::displayResult(const QString& result)
{
    m_output->setText(result);
}

void QalculateApplet::receivedResult(const QString& result)
{
    if (m_settings->resultsInline()) {
        m_input->setText(result);
    }

    if (m_settings->copyToClipboard()) {
        m_engine->copyToClipboard();
    }
}

void QalculateApplet::configChanged()
{
    if (m_settings->resultsInline()) {
        m_output->hide();
        m_layout->removeItem(m_output);
        m_graphicsWidget->resize(m_input->size());
    } else {
        m_output->show();
        m_layout->addItem(m_output);
        m_graphicsWidget->resize(m_graphicsWidget->preferredSize());
    }
    
    if (m_settings->liveEvaluation()) {
        connect(m_input, SIGNAL(textEdited(QString)), this, SLOT(evalNoHist()));
    }
    else {
        disconnect(m_input, SIGNAL(textEdited(QString)), this, SLOT(evalNoHist()));
    }

    Plasma::PopupApplet::configChanged();
}

void QalculateApplet::clearOutputLabel()
{
    if (m_input->text().isEmpty()) {
        m_output->nativeWidget()->setPixmap(KIcon("qalculate").pixmap(IconSize(KIconLoader::Desktop)));
    }
}

void QalculateApplet::giveFocus()
{
    m_graphicsWidget->setFocus();
    m_input->setFocus();
    m_input->nativeWidget()->setFocus();
}

void QalculateApplet::nextHistory()
{
    if (m_history->backup().isEmpty() && m_history->isAtEnd()) {
        m_history->setBackup(m_input->text());
    }
    
    m_input->setText(m_history->nextItem());
}

void QalculateApplet::previousHistory()
{
    if (m_history->backup().isEmpty() && m_history->isAtEnd()) {
        m_history->setBackup(m_input->text());
    }
    
    m_input->setText(m_history->previousItem());
}

#include "qalculate_applet.moc"
