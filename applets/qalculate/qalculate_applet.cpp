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
#include <KPushButton>
#include <KIcon>
#include <KAction>
#include <KConfigDialog>

#include <Plasma/LineEdit>
#include <Plasma/PushButton>
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
    m_engine = new QalculateEngine(m_settings, this);
    m_history = new QalculateHistory(this);
    connect(m_engine, SIGNAL(formattedResultReady(QString)), this, SLOT(displayResult(QString)));
    connect(m_engine, SIGNAL(resultReady(QString)), this, SLOT(createTooltip()));
    connect(m_engine, SIGNAL(resultReady(QString)), this, SLOT(receivedResult(QString)));
    connect(m_settings, SIGNAL(accepted()), this, SLOT(evalNoHist()));

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
    m_settings->readSettings();
    m_history->setHistoryItems(m_settings->historyItems());

    graphicsWidget();
    setupActions();
    setPopupIcon(KIcon("qalculate-applet"));
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
        m_graphicsWidget->setMinimumWidth(200);

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
        m_output->nativeWidget()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QFont f = m_output->nativeWidget()->font();
        f.setBold(true);
        f.setPointSize(resultSize() / 2);
        m_output->nativeWidget()->setFont(f);
        m_output->setFocusPolicy(Qt::NoFocus);
        connect(m_output, SIGNAL(clicked()), this, SLOT(giveFocus()));

        m_historyButton = new Plasma::PushButton;
        m_historyButton->setText(i18n("Show History"));
        m_historyButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        connect(m_historyButton->nativeWidget(), SIGNAL(clicked()), this, SLOT(showHistory()));

        m_historyList = new QGraphicsLinearLayout(Qt::Vertical);

        QPalette palette = m_output->palette();
        palette.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        m_output->nativeWidget()->setPalette(palette);

        m_layout = new QGraphicsLinearLayout(Qt::Vertical);
        m_layout->insertItem(0, m_input);
        m_layout->insertItem(1, m_output);
        if (!m_history->historyItems().isEmpty()) {
            m_layout->insertItem(2, m_historyButton);
        }

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
    data.setImage(KIcon("qalculate-applet").pixmap(IconSize(KIconLoader::Desktop)));
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
    m_settings->setHistoryItems(m_history->historyItems());
    if (!m_history->historyItems().isEmpty() && m_layout->itemAt(2) != m_historyButton) {
        m_layout->insertItem(2, m_historyButton);
    }
    hideHistory();
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
    m_settings->readSettings();
    if (m_settings->resultsInline()) {
        m_output->hide();
        m_layout->removeItem(m_output);
        m_graphicsWidget->resize(m_input->size());
    } else {
        m_output->show();
        m_layout->insertItem(1, m_output);
        m_graphicsWidget->resize(m_graphicsWidget->preferredSize());
    }

    if (m_settings->liveEvaluation()) {
        connect(m_input, SIGNAL(textEdited(QString)), this, SLOT(evalNoHist()), Qt::UniqueConnection);
    }
    else {
        disconnect(m_input, SIGNAL(textEdited(QString)), this, SLOT(evalNoHist()));
    }
}

void QalculateApplet::clearOutputLabel()
{
    if (m_input->text().isEmpty()) {
        m_output->nativeWidget()->setPixmap(KIcon("qalculate-applet").pixmap(IconSize(KIconLoader::Desktop)));
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

void QalculateApplet::showHistory()
{
    if (m_history->backup().isEmpty() && m_history->isAtEnd()) {
        m_history->setBackup(m_input->text());
    }

    if (m_historyButton->text() == i18n("Show History")) {
        QStringList itemList = m_history->historyItems();
        int i = itemList.size() - 1;

        //populate list
        for( ; i >= 0; i-- )
        {
            if (!itemList.at(i).isEmpty()) {
                OutputLabel *item = new OutputLabel;
                item->setText(itemList.at(i));
                m_historyList->addItem(item);
            }
        }

        m_historyButton->setText(i18n("Hide History"));
        m_layout->addItem(m_historyList);
    }
    else {
        hideHistory();
    }
}

void QalculateApplet::hideHistory()
{
    if (m_historyButton->text() == i18n("Hide History")) {
        //clear historyList
        while (m_historyList->count() != 0) {
            QGraphicsLayoutItem *item;
            item = m_historyList->itemAt(0);
            m_historyList->removeItem(item);
            delete item;
        }
    }

    m_layout->removeItem(m_historyList);
    m_historyButton->setText(i18n("Show History"));
    m_graphicsWidget->resize(m_graphicsWidget->preferredSize());
}

#include "qalculate_applet.moc"
