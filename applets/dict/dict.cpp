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
//#include "arrow.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
//#include <QTextArea>
#include <QLineEdit>
#include <iostream>


#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>
#include <QTime>
#include <plasma/svg.h>
#include <plasma/phase.h>

using namespace Plasma;

Dict::Dict(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0),
      m_flash(0)
{
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);

    KConfigGroup cg = config();
    m_width = cg.readEntry("width", 500);
    m_autoDefineTimeout = cg.readEntry("autoDefineTimeout", 500);
    m_wordEdit = new Plasma::LineEdit(this);
    m_wordEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_wordEdit->setDefaultText(i18n("Enter word to define here"));
    Phase::self()->animateItem(m_wordEdit, Phase::Appear);
    m_defEdit = new Plasma::LineEdit(this);
    m_defEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);

    m_defEdit->hide();
    m_defEdit->setMultiLine(true);
    //m_wordEdit->setZValue(m_defEdit->zValue()+1);

//  Icon in upper-left corner
    QIcon icon = KIcon("accessories-dictionary");
    m_graphicsIcon = new QGraphicsPixmapItem(icon.pixmap(32,32), this);

//  Position lineedits
    const int wordEditOffset = 40;
    m_graphicsIcon->setPos(-40 + wordEditOffset + 12,3);
    m_wordEdit->setPos(15 + wordEditOffset,7);
    m_wordEdit->setTextWidth(m_width-wordEditOffset);
    m_defEdit->setTextWidth(m_width);
    m_defEdit->setPos(15,40);

//  Timer for auto-define
    m_timer = new QTimer(this);
    m_timer->setInterval(m_autoDefineTimeout);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(define()));
    
    m_word = QString("");
    dataEngine("dict")->connectSource(m_word, this);
    connect(m_wordEdit, SIGNAL(editingFinished()), this, SLOT(define()));
    connect(m_wordEdit, SIGNAL(textChanged(const QString&)), this, SLOT(autoDefine(const QString&)));
    connect(m_defEdit, SIGNAL(linkActivated(const QString&)), this, SLOT(linkDefine(const QString&)));
    
//  This is the fix for links/selecting text
    QGraphicsItem::GraphicsItemFlags flags = m_defEdit->flags();
    flags ^= QGraphicsItem::ItemIsMovable;
    m_defEdit->setFlags(flags);

//  Setup Arrows
    m_rightArrow = new Arrow(this);
    m_rightArrow->setPos(contentSize().width()-m_rightArrow->boundingRect().width(),contentSize().height());
    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(pageRight()));
    m_rightArrow->setDirection(1);
    m_rightArrow->setZValue(1);
    m_rightArrow->hide();
    m_leftArrow = new Arrow(this);
    m_leftArrow->setPos(0,contentSize().height());
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(pageLeft()));
    m_leftArrow->setDirection(0);
    m_leftArrow->setZValue(1);
    m_leftArrow->hide();

    m_flash = new Plasma::Flash( this );
    m_flash->setColor( Qt::gray );
    QFont fnt = qApp->font();
    fnt.setBold( true );
    m_flash->setFont( fnt );
    m_flash->setPos(25,-10);
    m_flash->setSize(QSize(200,20));
}

void Dict::pageRight()
{
    m_leftArrow->show();
    kDebug()<< "right\n";
    if (m_i != --m_defList.end())
        m_i++;
    m_defEdit->setHtml(*m_i);
    if (m_i == --m_defList.end())
        m_rightArrow->hide();
    updateGeometry();
}

void Dict::pageLeft()
{
    m_rightArrow->show();
    kDebug()<< "left\n";
    if (m_i != m_defList.begin())
        m_i--;
    m_defEdit->setHtml(*m_i);
    if (m_i == m_defList.begin())
        m_leftArrow->hide();
    updateGeometry();
}

void Dict::linkDefine(const QString &text)
{
    kDebug() <<"ACTIVATED\n";
    m_wordEdit->setPlainText(text);
    define();
}

QSizeF Dict::contentSizeHint() const
{
    if (m_defEdit->isVisible()) {
        return QSizeF(30 + m_defEdit->boundingRect().width(), 50 + m_defEdit->boundingRect().height());
    } else {
        return QSizeF(30 + m_wordEdit->boundingRect().width() + 40, 40);
    }
}

void Dict::constraintsUpdated(Plasma::Constraints)
{
    updateGeometry();
}

void Dict::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    if (m_flash) {
      m_flash->kill();
    }
    if (!m_word.isEmpty()) {
        m_defEdit->show();
        Phase::self()->animateItem(m_defEdit, Phase::Appear);
    }
    if (data.contains("gcide")) {
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
        //m_defEdit->updated(QString("test"),data);
    }
    updateGeometry();
}

void Dict::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Dict") );
	ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.widthSpinBox->setValue(m_width);
    ui.timeoutSpinBox->setValue(m_autoDefineTimeout);
    m_dialog->show();
    kDebug() << "SHOW config dialog";
}

void Dict::define()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    QString newWord = m_wordEdit->toPlainText();

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
        m_defEdit->setPlainText(QString());
        Phase::self()->animateItem(m_defEdit, Phase::Disappear);
        m_defEdit->hide();
        m_rightArrow->hide();
        m_leftArrow->hide();
    }

    updateConstraints();
}

void Dict::configAccepted()
{
    KConfigGroup cg = config();

    m_width = ui.widthSpinBox->value();
    cg.writeEntry("width", m_width);

    m_autoDefineTimeout = ui.timeoutSpinBox->value();
    cg.writeEntry("autoDefineTimeout", m_autoDefineTimeout);
    m_timer->setInterval(m_autoDefineTimeout);

    m_dialog->deleteLater();
    m_dialog = 0;

    // these are automatically deallocated when we delete m_dialog
    // zero them out just to be safe and conscientious
    updateConstraints();
}

Dict::~Dict()
{
    delete m_dialog;
}

void Dict::autoDefine(const QString &word)
{
    Q_UNUSED(word)
    m_timer->start();
}

#include "dict.moc"
