/*
    Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net> 
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of        
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
    GNU General Public License for more details.                          
                                                                          
    You should have received a copy of the GNU General Public License     
    along with this program; if not, write to the                         
    Free Software Foundation, Inc.,                                       
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        
 */

#include "notes.h"

#include <KDialog>
#include <KConfigGroup>
#include <KFontDialog>
#include <KColorDialog>

#include <QFont>
#include <QPainter>
#include <QRectF>
#include <QSizeF>
#include <QGraphicsTextItem>
#include <QIntValidator>

#include <plasma/widgets/vboxlayout.h>

Notes::Notes(QObject *parent,
             const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_notes_theme("widgets/notes", this),
    m_dialog(0),
    m_size(256,256)

{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);


    m_notes_theme.setContentType(Plasma::Svg::SingleImage);

    m_textArea = new Plasma::LineEdit(this);
    m_textArea->setMultiLine(true);

    cg = config();

    m_textArea->setTextWidth(cg.readEntry("textWidth",205));
    m_textArea->setPos(25, 25);
    m_textArea->setPlainText(cg.readEntry("autoSave",i18n("Welcome to Notes Plasmoid! Type your notes here...")));
    m_textArea->setStyled(false);
    m_textArea->setOpenExternalLinks(true);
    m_textArea->setFont(cg.readEntry("font",m_font));
    m_textArea->setDefaultTextColor(cg.readEntry("textcolor",m_textColor));
}

void Notes::saveNote()
{
    KConfigGroup cg = config();
    cg.writeEntry("autoSave",m_textArea->toPlainText());
    cg.config()->sync();

}

Notes::~Notes()
{
    saveNote();
}

void Notes::setContentSize(const QSizeF& size)
{
    m_size = size;
}

QSizeF Notes::contentSizeHint() const
{
    return m_size;
}

void Notes::paintInterface(QPainter *p,
                           const QStyleOptionGraphicsItem *option,
                           const QRect &contentsRect)
{
    Q_UNUSED(option);


    m_notes_theme.resize((int)contentsRect.width(),
                         (int)contentsRect.height());
    m_notes_theme.paint(p,
                       (int)contentsRect.left(),
                       (int)contentsRect.top());
}

void Notes::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Notes Configuration") );
        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        m_dialog->showButtonSeparator(true);
        ui.sizeEdit->setValidator(new QIntValidator(0,2048, ui.sizeEdit));
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
        connect( ui.fontSelectButton, SIGNAL(clicked()), this, SLOT(showFontSelectDlg()) );
        connect( ui.colorSelectButton, SIGNAL(clicked()), this, SLOT(showColorSelectDlg()));
    }

    m_dialog->show();
}

void Notes::showFontSelectDlg()
{
    KFontDialog::getFont(m_font);
}

void Notes::showColorSelectDlg()
{
    KColorDialog::getColor( m_textColor);
}

void Notes::configAccepted()
{
    prepareGeometryChange();

    cg.writeEntry("size", ui.sizeEdit->text());
    cg.writeEntry("font", m_font);
    cg.writeEntry("textcolor", m_textColor);
    cg.writeEntry("savenote", ui.saveIfClosed->isChecked() ? "closenote" : "focusleft" );
    cg.config()->sync();

    QSizeF size(ui.sizeEdit->text().toInt(),ui.sizeEdit->text().toInt());
    setContentSize(size);
    m_textArea->setFont(m_font);
    m_textArea->setDefaultTextColor(m_textColor);
}

#include "notes.moc"
