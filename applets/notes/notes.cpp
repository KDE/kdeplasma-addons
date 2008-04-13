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

Notes::Notes(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_notes_theme("widgets/notes", this),
    m_dialog(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setDrawStandardBackground(false);
    resize(256, 256);
}

void Notes::init()
{

    m_notes_theme.setContentType(Plasma::Svg::SingleImage);

    m_textArea = new Plasma::LineEdit(this);
    m_textArea->setMultiLine(true);

    KConfigGroup cg = config();

    updateTextGeometry();
    m_textArea->setDefaultText(i18n("Welcome to Notes Plasmoid! Type your notes here..."));
    QString text = cg.readEntry("autoSave",QString());
    if (! text.isEmpty()) {
        m_textArea->setPlainText(text);
    }
    m_textArea->setStyled(false);
    //FIXME this has no effect right now. try setTextInteractionFlags
    m_textArea->setOpenExternalLinks(true);
    QFont font = cg.readEntry("font", QFont());
    m_textArea->setFont(font);
    QColor textColor = cg.readEntry("textcolor", QColor(Qt::black));
    m_textArea->setDefaultTextColor(textColor);
    connect(m_textArea, SIGNAL(editingFinished()), this, SLOT(saveNote())); // FIXME: Doesn't work? This could make the following unnecessary ...
}

void Notes::constraintsUpdated(Plasma::Constraints constraints)
{
    //XXX why does everything break so horribly if I remove this line?
    setDrawStandardBackground(false);
    if (constraints & Plasma::SizeConstraint) {
        updateTextGeometry();
    }
}

void Notes::updateTextGeometry()
{
    //note: we're using a custom bg so we have no 'border': using boundingrect here is ok
    //FIXME there's no way to force the height on a qgraphicstextitem :(
    const qreal xpad = boundingRect().width() / 10;
    const qreal ypad = boundingRect().height() / 10;
    m_textArea->setGeometry(QRectF(xpad, ypad, boundingRect().width() - 2 * xpad, boundingRect().height() - 2 * ypad));
}

void Notes::saveNote()
{
    KConfigGroup cg = config();
    cg.writeEntry("autoSave", m_textArea->toPlainText());
    emit configNeedsSaving();
}

Notes::~Notes()
{
    saveNote();
    //FIXME is it really ok to save from here?
    //also, this has a really weird effect: if I remove a note then add a new one, I can get the old
    //text back. it was useful when there were load/save issues but it's silly now.
    delete m_dialog;
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
        m_dialog->setWindowIcon(KIcon("knotes"));
        m_dialog->setCaption( i18n("Notes Configuration") );
        ui.setupUi(m_dialog->mainWidget());
        m_dialog->mainWidget()->layout()->setMargin(0);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.textColorButton->setColor(m_textArea->defaultTextColor());
    ui.textFontButton->setFont(m_textArea->font());
    m_dialog->show();
}

void Notes::configAccepted()
{
    prepareGeometryChange();

    KConfigGroup cg = config();

    bool changed = false;

    QFont newFont = ui.textFontButton->font();
    if (m_textArea->font() != newFont) {
        changed = true;
        cg.writeEntry("font", newFont);
        m_textArea->setFont(newFont);
    }

    QColor newColor = ui.textColorButton->color();
    if (m_textArea->defaultTextColor() != newColor) {
        changed = true;
        cg.writeEntry("textcolor", newColor);
        m_textArea->setDefaultTextColor(newColor);
    }

    if (changed) {
        emit configNeedsSaving();
    }
}

#include "notes.moc"
