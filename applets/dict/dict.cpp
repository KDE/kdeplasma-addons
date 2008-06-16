/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *   Copyright (C) 2007 by Jeff Cooper <weirdsox11@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "dict.h"

#include <QGraphicsProxyWidget>
#include <QTime>
#include <QTimer>
#include <QtWebKit/QWebView>
#include <QGraphicsLinearLayout>

#include <KConfigDialog>
#include <KDebug>
#include <KIcon>
#include <KLineEdit>

#include <Plasma/Animator>
#include <plasma/widgets/icon.h>

using namespace Plasma;

Dict::Dict(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
      //m_flash(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    resize(500,200);
}

void Dict::init()
{
    KConfigGroup cg = config();

    m_autoDefineTimeout = cg.readEntry("autoDefineTimeout", 500);
    m_wordProxyWidget = new QGraphicsProxyWidget(this);
    m_wordEdit = new KLineEdit;
    m_wordEdit->setClearButtonShown( true );
    m_wordEdit->setClickMessage(i18n("Enter word to define here"));
    m_wordEdit->setAttribute(Qt::WA_NoSystemBackground);
    m_wordProxyWidget->setWidget(m_wordEdit);
    m_wordProxyWidget->show();
    Plasma::Animator::self()->animateItem(m_wordProxyWidget, Plasma::Animator::AppearAnimation);

    m_defBrowser = new QWebView();
    m_defBrowserProxy = new QGraphicsProxyWidget(this);
    m_defBrowserProxy->setWidget(m_defBrowser);
    m_defBrowserProxy->hide();
//  Icon in upper-left corner
    QIcon icon = KIcon("accessories-dictionary");
    //m_graphicsIcon = new QGraphicsPixmapItem(icon.pixmap(32,32), this);
    m_icon = new Plasma::Icon(this);
    m_icon->setIcon(icon);

//  Position lineedits
    //const int wordEditOffset = 40;
    m_icon->setPos(12,3);
    //m_wordProxyWidget->setPos(15 + wordEditOffset,7);
    //m_wordProxyWidget->show();
    // TODO m_wordEdit->setDefaultTextColor(Plasma::Theme::self()->color(Plasma::Theme::TextColor));

//  Timer for auto-define
    m_timer = new QTimer(this);
    m_timer->setInterval(m_autoDefineTimeout);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(define()));

    m_horLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_horLayout->addItem(m_icon);
    m_horLayout->addItem(m_wordProxyWidget);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_horLayout);
    m_layout->addItem(m_defBrowserProxy);
    setLayout(m_layout);

    m_word = QString("");
    dataEngine("dict")->connectSource(m_word, this);
    connect(m_wordEdit, SIGNAL(editingFinished()), this, SLOT(define()));
    connect(m_wordEdit, SIGNAL(textChanged(const QString&)), this, SLOT(autoDefine(const QString&)));
    //connect(m_defEdit, SIGNAL(linkActivated(const QString&)), this, SLOT(linkDefine(const QString&)));

//  This is the fix for links/selecting text
    //QGraphicsItem::GraphicsItemFlags flags = m_defEdit->flags();
    //flags ^= QGraphicsItem::ItemIsMovable;
   // m_defEdit->setFlags(flags);

    /*m_flash = new Plasma::Flash(this);
    m_flash->setColor(Qt::gray);
    QFont fnt = qApp->font();
    fnt.setBold(true);
    m_flash->setFont(fnt);
    m_flash->setPos(25,-10);
    m_flash->resize(QSize(200,20));*/
}


void Dict::linkDefine(const QString &text)
{
    kDebug() <<"ACTIVATED";
    m_wordEdit->setText(text);
    define();
}

void Dict::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        //updateGeometry();
    }
    if (constraints & Plasma::SizeConstraint) {
        updateGeometry();
    }
}

void Dict::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    /*if (m_flash) {
        m_flash->kill();
    }*/
    if (!m_word.isEmpty()) {
        m_defBrowserProxy->show();
        // TODO Phase::self()->animateItem(m_defBrowserProxy, Phase::Appear);
    }
/*    if (data.contains("gcide")) {
        QString defHeader;
        m_defList = data[QString("gcide")].toString().split("<!--PAGE START-->"); //<!--DEFINITION START-->
        for (int n = 0; n < m_defList.size(); ++n)
        {
            if (m_defList[n].contains("<!--DEFINITION START-->") && m_defList[n].contains("<!--PERIOD-->")) {
                defHeader=m_defList[n];
            } else if (m_defList[n].contains("<!--DEFINITION START-->")) {
                defHeader=m_defList.takeAt(n);
            }
            if (n < m_defList.size() && !m_defList[n].contains("<!--DEFINITION START-->"))
                m_defList[n].prepend(defHeader);
        }
        if (m_defList.size() > 1)
            m_defList.removeAt(0);
        m_i = m_defList.begin();
        m_defEdit->setHtml(*m_i);
        if (m_i != --m_defList.end())
            m_rightArrow->show();
        else
            m_rightArrow->hide();
        m_leftArrow->hide();
    } */
    if (data.contains("wn")) {
        m_defBrowser->setHtml(wnToHtml(data[QString("wn")].toString()));
    }
    updateGeometry();
}

QString Dict::wnToHtml(const QString &text)
{
    QList<QString> splitText = text.split('\n');
    QString def;
    def += "<dl>\n";
    bool isFirst=true;
    while (!splitText.empty()) {
        QString currentLine = splitText.takeFirst();
        if (currentLine.startsWith("151")) {
            isFirst = true;
            continue;
        }
        if (currentLine.startsWith('.')) {
            def += "</dd>";
            continue;
        }
        if (!(currentLine.startsWith("150") || currentLine.startsWith("151")
           || currentLine.startsWith("250") || currentLine.startsWith("552"))) {
            if (isFirst) {
                def += "<dt><b>" + currentLine + "</b></dt>\n<dd>";
                isFirst = false;
                continue;
            } else {
                if (currentLine.contains(QRegExp("([1-9]{1,2}:)"))) {
                    def += "\n<br>\n";
                }
                currentLine.replace(QRegExp("^([\\s\\S]*[1-9]{1,2}:)"), "<b>\\1</b>");
                def += currentLine;
                continue;
            }
        }

    }
    def += "</dl>";
    return def;
}

void Dict::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), icon());

    ui.timeoutSpinBox->setValue(m_autoDefineTimeout);
    kDebug() << "SHOW config dialog";
}

void Dict::define()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    QString newWord = m_wordEdit->text();

    if (newWord == m_word) {
        // avoid re-defining the same word
        return;
    }

    dataEngine("dict")->disconnectSource(m_word, this);
    m_word = newWord;

    if (!m_word.isEmpty()) { //get new definition
        //m_flash->flash(i18n("Looking up ") + m_word);
        dataEngine("dict")->connectSource(m_word, this);
    } else { //make the definition box disappear
        // TODO Phase::self()->animateItem(m_defBrowserProxy, Phase::Disappear);
        m_defBrowserProxy->hide();
    }

    updateConstraints();
}

void Dict::configAccepted()
{
    KConfigGroup cg = config();

    m_autoDefineTimeout = ui.timeoutSpinBox->value();
    cg.writeEntry("autoDefineTimeout", m_autoDefineTimeout);
    m_timer->setInterval(m_autoDefineTimeout);

    updateConstraints();

    emit configNeedsSaving();
}

Dict::~Dict()
{
}

void Dict::autoDefine(const QString &word)
{
    Q_UNUSED(word)
    m_timer->start();
}

#include "dict.moc"
