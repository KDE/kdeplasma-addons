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

#include <math.h>

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QStyleOptionGraphicsItem>
#include <QCheckBox>
#include <QPushButton>
#include <QTime>
#include <QTimer>
#include <QtWebKit/QWebView>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>

#include <iostream>

#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>

#include <plasma/svg.h>
#include <plasma/animator.h>
#include <plasma/theme.h>

using namespace Plasma;

Dict::Dict(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_flash(0)
{
    setHasConfigurationInterface(true);
    resize(500,200);
}

void Dict::init()
{
    KConfigGroup cg = config();

    m_autoDefineTimeout = cg.readEntry("autoDefineTimeout", 500);
    m_lineProxyWidget = new QGraphicsProxyWidget(this);
    m_wordEdit = new KLineEdit;
    m_lineProxyWidget->setWidget(m_wordEdit);

    //TODO m_wordEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_wordEdit->setText(i18n("Enter word to define here"));
    //TODO Phase::self()->animateItem(m_wordEdit, Phase::Appear);  

    m_defBrowser = new QWebView();
    m_defDisplayProxy = new QGraphicsProxyWidget(this);
    m_defDisplayProxy->setWidget(m_defBrowser);
    //m_defDisplayProxy->setPos(8,40);
    //m_defDisplayProxy->resize(200,200);
    m_defDisplayProxy->hide();
//  Icon in upper-left corner
    QIcon icon = KIcon("accessories-dictionary");
    m_graphicsIcon = new QGraphicsPixmapItem(icon.pixmap(32,32), this);

//  Position lineedits
    const int wordEditOffset = 40;
    m_graphicsIcon->setPos(12,3);
    m_lineProxyWidget->setPos(15 + wordEditOffset,7);
    m_lineProxyWidget->show();
    // TODO m_wordEdit->setTextWidth(contentSize().width()-wordEditOffset-10);

    // TODO m_wordEdit->setStyled(true);
    // TODO m_wordEdit->setDefaultTextColor(Plasma::Theme::self()->textColor());

//  Timer for auto-define
    m_timer = new QTimer(this);
    m_timer->setInterval(m_autoDefineTimeout);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(define()));

    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_lineProxyWidget);
    m_layout->addItem(m_defDisplayProxy);
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

    m_flash = new Plasma::Flash(this);
    m_flash->setColor(Qt::gray);
    QFont fnt = qApp->font();
    fnt.setBold(true);
    m_flash->setFont(fnt);
    m_flash->setPos(25,-10);
    m_flash->resize(QSize(200,20));
}


void Dict::linkDefine(const QString &text)
{
    kDebug() <<"ACTIVATED";
    m_wordEdit->setText(text);
    define();
}

QSizeF Dict::contentSizeHint() const
{
//      if (m_defEdit->isVisible()) {
//          return QSizeF(contentSize().width(), 50 + m_defEdit->boundingRect().height());
//      } else {
//          return QSizeF(contentSize().width(), 40);
//      }
      if (m_defDisplayProxy->isVisible()) {
          return QSizeF(geometry().width(), 40+m_defDisplayProxy->size().height());
      } else { 
         return QSizeF(geometry().width(), 40);
      }
//      return QSizeF(contentSize());
}

void Dict::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        //updateGeometry();
    }
    if (constraints & Plasma::SizeConstraint) {
    if (m_defDisplayProxy->isVisible()) {
            m_defDisplayProxy->resize(geometry().width()-15, geometry().height()-40);
    }
        //m_wordEdit->setTextWidth(geometry().width()-60);
        updateGeometry();
    }
}

void Dict::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    if (m_flash) {
        m_flash->kill();
    }
    if (!m_word.isEmpty()) {
        m_defDisplayProxy->show();
        // TODO Phase::self()->animateItem(m_defDisplayProxy, Phase::Appear);
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
        if (currentLine.startsWith(".")) {
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
        m_flash->flash(i18n("Looking up ") + m_word);
        dataEngine("dict")->connectSource(m_word, this);
    } else { //make the definition box disappear
        // TODO Phase::self()->animateItem(m_defDisplayProxy, Phase::Disappear);
        m_defDisplayProxy->hide();
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
