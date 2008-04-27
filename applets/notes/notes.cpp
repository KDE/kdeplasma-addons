/***************************************************************************
 *   Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net>         *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
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

#include "notes.h"

#include <QTextEdit>
#include <QGraphicsLinearLayout>
#include <QGraphicsTextItem>

#include <KConfigDialog>
#include <KConfigGroup>
#include <KFontDialog>
#include <KColorDialog>

Notes::Notes(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_notes_theme(this)
{
    m_notes_theme.setImagePath("widgets/notes");
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setBackgroundHints(Plasma::Applet::NoBackground);
    resize(256, 256);

    m_textEdit = new QTextEdit();
    m_layout = new QGraphicsLinearLayout();
    m_proxy = new QGraphicsProxyWidget(this);
    updateTextGeometry();
}

void Notes::init()
{
    m_notes_theme.setContainsMultipleImages(false);

    m_proxy->setWidget(m_textEdit);
    m_proxy->show();
    m_textEdit->setStyleSheet("QTextEdit { border: none }");
    m_textEdit->setAttribute(Qt::WA_NoSystemBackground);
    m_textEdit->setTextBackgroundColor(QColor(0,0,0,0));
    m_textEdit->viewport()->setAutoFillBackground(false);
    m_layout->addItem(m_proxy);

    KConfigGroup cg = config();

    m_textEdit->setPlainText(i18n("Welcome to the Notes Plasmoid! Type your notes here..."));
    QString text = cg.readEntry("autoSave",QString());
    if (! text.isEmpty()) {
        m_textEdit->setPlainText(text);
    }
    QFont font = cg.readEntry("font", QFont());
    m_textEdit->setFont(font);
    QColor textColor = cg.readEntry("textcolor", QColor(Qt::black));
    m_textEdit->setTextColor(textColor);

    setLayout(m_layout);
    updateTextGeometry();
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(saveNote()));
}

void Notes::constraintsEvent(Plasma::Constraints constraints)
{
    //XXX why does everything break so horribly if I remove this line?
    setBackgroundHints(Plasma::Applet::NoBackground);
    if (constraints & Plasma::SizeConstraint) {
        updateTextGeometry();
    }
}

void Notes::updateTextGeometry()
{
    const qreal xpad = geometry().width() / 10;
    const qreal ypad = geometry().height() / 10;
    m_layout->setSpacing(xpad);
    m_layout->setContentsMargins(xpad, ypad, xpad, ypad);
}

void Notes::saveNote()
{
    KConfigGroup cg = config();
    cg.writeEntry("autoSave", m_textEdit->toPlainText());
    kDebug() << m_textEdit->toPlainText();
    emit configNeedsSaving();
}

Notes::~Notes()
{
    emit configNeedsSaving();
    //FIXME is it really ok to save from here?
    //also, this has a really weird effect: if I remove a note then add a new one, I can get the old
    //text back. it was useful when there were load/save issues but it's silly now.
    saveNote();
}

void Notes::paintInterface(QPainter *p,
                           const QStyleOptionGraphicsItem *option,
                           const QRect &contentsRect)
{
    Q_UNUSED(option);

    m_notes_theme.resize(geometry().size());
    m_notes_theme.paint(p, contentsRect);
}

void Notes::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), "notes");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.textColorButton->setColor(m_textEdit->textColor());
    ui.textFontButton->setFont(m_textEdit->font());
}

void Notes::configAccepted()
{
    prepareGeometryChange();
    KConfigGroup cg = config();
    bool changed = false;

    QFont newFont = ui.textFontButton->font();
    if (m_textEdit->font() != newFont) {
        changed = true;
        cg.writeEntry("font", newFont);
        m_textEdit->setFont(newFont);
    }

    QColor newColor = ui.textColorButton->color();
    kDebug() << m_textEdit->textColor() << newColor;
    if (m_textEdit->textColor() != newColor) {
        changed = true;
        cg.writeEntry("textcolor", newColor);
        m_textEdit->setTextColor(newColor);
    }

    if (changed) {
        emit configNeedsSaving();
    }
}

#include "notes.moc"
