/***************************************************************************
 *   Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net>         *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2008 Davide Bettio <davide.bettio@kdemail.net>
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

#include <QFile>
#include <QGraphicsLinearLayout>
#include <QGraphicsTextItem>
#include <QMenu>
#include <QScrollBar>
#include <QTextStream>

#include <KAction>
#include <KFileDialog>
#include <KMessageBox>
#include <KGlobalSettings>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KStandardAction>
#include <KTextEdit>

#include <Plasma/TextEdit>
NotesTextEdit::NotesTextEdit( QWidget *parent ): KTextEdit( parent )
{
}

NotesTextEdit::~NotesTextEdit()
{
}

/**
 * Customize the context menu
 */
void NotesTextEdit::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *popup = mousePopupMenu();
    popup->addSeparator();
    popup->addAction(KStandardAction::saveAs(this, SLOT(saveToFile()), this));
    popup->exec(event->globalPos());
    delete popup;
}

/**
 * Use notesTextEdit as native widget instead of KTextEdit
 */
PlasmaTextEdit::PlasmaTextEdit(QGraphicsWidget *parent)
    : Plasma::TextEdit(parent)
{
    KTextEdit *w = nativeWidget();
    NotesTextEdit* native = new NotesTextEdit;
    //FIXME: we need a way to just add actions without changing the native widget under its feet
    if (native->verticalScrollBar() && w->verticalScrollBar()) {
        native->verticalScrollBar()->setStyle(w->verticalScrollBar()->style());
    }
    connect(native, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    setWidget(native);
    delete w;
    native->setAttribute(Qt::WA_NoSystemBackground);

}

PlasmaTextEdit::~PlasmaTextEdit()
{
}

/**
 * Save content to file
 */
void NotesTextEdit::saveToFile()
{
    QString fileName = KFileDialog::getSaveFileName();

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        KMessageBox::information(this, file.errorString(), i18n("Unable to open file"));
        return;
    }

    QTextStream out(&file);
    out << toPlainText();
    file.close();
}

Notes::Notes(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_notes_theme(this),
      m_layout(0),
      m_textEdit(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
    setBackgroundHints(Plasma::Applet::NoBackground);
    m_saveTimer.setSingleShot(true);
    connect(&m_saveTimer, SIGNAL(timeout()), this, SLOT(saveNote()));
    resize(256, 256);

    m_textEdit = new PlasmaTextEdit(this);
    m_textEdit->setMinimumSize(QSize(0, 0));
    m_layout = new QGraphicsLinearLayout(this);
    m_textEdit->nativeWidget()->setFrameShape(QFrame::NoFrame);
    m_textEdit->nativeWidget()->viewport()->setAutoFillBackground(false);
    m_layout->addItem(m_textEdit);

    if (args.count() > 0) {
        QFile f(args.at(0).toString());

        if (f.open(QIODevice::ReadOnly)) {
            QTextStream t(&f);
            m_textEdit->nativeWidget()->setPlainText(t.readAll());
            f.close();
        }
    } else {
#ifdef KTEXTEDIT_CLICKMSG_SUPPORT
        m_textEdit->nativeWidget()->setClickMessage(i18n("Welcome to the Notes Plasmoid! Type your notes here..."));
#endif
    }
}

Notes::~Notes()
{
    saveNote();
    delete m_textEdit;
}

void Notes::init()
{
    m_notes_theme.setImagePath("widgets/notes");
    m_notes_theme.setContainsMultipleImages(false);

    addColor("white", i18n("White"));
    addColor("black", i18n("Black"));
    addColor("red", i18n("Red"));
    addColor("orange", i18n("Orange"));
    addColor("yellow", i18n("Yellow"));
    addColor("green", i18n("Green"));
    addColor("blue", i18n("Blue"));
    addColor("pink", i18n("Pink"));
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    m_colorActions.append(separator);

    m_autoFont = false;

    KConfigGroup cg = config();
    m_color = cg.readEntry("color", "yellow");
    // color must be before setPlainText("foo")
    m_textColor = cg.readEntry("textcolor", QColor(Qt::black));
    m_textEdit->nativeWidget()->setTextColor(m_textColor);

    QString text = cg.readEntry("autoSave", QString());
    if (!text.isEmpty()) {
        m_textEdit->nativeWidget()->setPlainText(text);
    }

    int scrollValue = cg.readEntry("scrollValue").toInt();
    if (scrollValue) {
        m_textEdit->nativeWidget()->verticalScrollBar()->setValue(scrollValue);
    }

    m_font = cg.readEntry("font", KGlobalSettings::generalFont());
    m_customFontSize = cg.readEntry("customFontSize", m_font.pointSize());
    m_autoFont = cg.readEntry("autoFont", true);
    m_autoFontPercent = cg.readEntry("autoFontPercent", 4);

    m_useThemeColor = cg.readEntry("useThemeColor", true);

    m_checkSpelling = cg.readEntry("checkSpelling", false);
    m_textEdit->nativeWidget()->setCheckSpellingEnabled(m_checkSpelling);
    updateTextGeometry();
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(delayedSaveNote()));
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
    if (m_layout) {
        const qreal xpad = geometry().width() / 15;
        const qreal ypad = geometry().height() / 15;
        m_layout->setSpacing(xpad);
        m_layout->setContentsMargins(xpad, ypad, xpad, ypad);
        m_font.setPointSize(fontSize());
        m_textEdit->nativeWidget()->setFont(m_font);
    }
}

int Notes::fontSize()
{
    if (m_autoFont) {
        int autosize = qRound(((geometry().width() + geometry().height())/2)*m_autoFontPercent/100);
        return qMax(KGlobalSettings::smallestReadableFont().pointSize(), autosize);
    } else {
        return m_customFontSize;
    }
}

void Notes::delayedSaveNote()
{
    m_saveTimer.start(5000);
}

void Notes::saveNote()
{
    KConfigGroup cg = config();
    cg.writeEntry("autoSave", m_textEdit->nativeWidget()->toPlainText());
    cg.writeEntry("scrollValue", QVariant(m_textEdit->nativeWidget()->verticalScrollBar()->value()));
    //kDebug() << m_textEdit->nativeWidget()->toPlainText();
    emit configNeedsSaving();
}

void Notes::addColor(const QString &id, const QString &colorName)
{
    QAction *tmpAction = new QAction(colorName, this);
    m_colorActions.append(tmpAction);
    tmpAction->setProperty("color", id);
    connect(tmpAction, SIGNAL(triggered(bool)), this, SLOT(changeColor()));
}

void Notes::changeColor()
{
    QAction *action = dynamic_cast<QAction*> (sender());
    if (!action || action->property("color").type() != QVariant::String) return;
    m_color = action->property("color").toString();
    update();
}

QList<QAction*> Notes::contextualActions()
{
    return m_colorActions;
}

void Notes::paintInterface(QPainter *p,
                           const QStyleOptionGraphicsItem *option,
                           const QRect &contentsRect)
{
    Q_UNUSED(option);

    m_notes_theme.resize(geometry().size());
    m_notes_theme.paint(p, contentsRect, m_color + "-notes");
}

void Notes::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, i18n("General"), "notes");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QButtonGroup *fontSizeGroup = new QButtonGroup(widget);
    fontSizeGroup->addButton(ui.autoFont);
    fontSizeGroup->addButton(ui.customFont);

    ui.textColorButton->setColor(m_textColor);
    ui.fontStyleComboBox->setCurrentFont(m_textEdit->nativeWidget()->font());
    ui.fontBoldCheckBox->setChecked(m_textEdit->nativeWidget()->font().bold());
    ui.fontItalicCheckBox->setChecked(m_textEdit->nativeWidget()->font().italic());
    ui.autoFont->setChecked(m_autoFont);
    ui.autoFontPercent->setEnabled(m_autoFont);
    ui.customFont->setChecked(!m_autoFont);
    ui.customFontSizeSpinBox->setEnabled(!m_autoFont);
    ui.autoFontPercent->setValue(m_autoFontPercent);
    ui.customFontSizeSpinBox->setValue(m_customFontSize);
    ui.useThemeColor->setChecked(m_useThemeColor);
    ui.useCustomColor->setChecked(!m_useThemeColor);
    ui.checkSpelling->setChecked(m_checkSpelling);

    for (int i = 0; i < m_colorActions.size(); i++){
        QString text = m_colorActions[i]->text();
        if (!text.isEmpty()){
            ui.notesColorComboBox->insertItem(i, text);
            if (m_colorActions[i]->property("color").toString() == m_color)
                ui.notesColorComboBox->setCurrentIndex(i);
        }
    }
}

void Notes::configAccepted()
{
    KConfigGroup cg = config();
    bool changed = false;

    QFont newFont = ui.fontStyleComboBox->currentFont();
    newFont.setBold(ui.fontBoldCheckBox->isChecked());
    newFont.setItalic(ui.fontItalicCheckBox->isChecked());
    if (m_font != newFont) {
        changed = true;
        cg.writeEntry("font", newFont);
        m_font = newFont;
        m_font.setPointSize(fontSize());
        m_textEdit->nativeWidget()->setFont(newFont);
    }

    if (m_customFontSize != ui.customFontSizeSpinBox->value()) {
        changed = true;
        m_customFontSize = ui.customFontSizeSpinBox->value();
        cg.writeEntry("customFontSize", m_customFontSize);
        m_font.setPointSize(fontSize());
    }

    if (m_autoFont != ui.autoFont->isChecked()) {
        changed = true;
        m_autoFont = ui.autoFont->isChecked();
        cg.writeEntry("autoFont", m_autoFont);
        m_font.setPointSize(fontSize());
    }

    if (m_autoFontPercent != ui.autoFontPercent->value()) {
        changed = true;
        m_autoFontPercent = (ui.autoFontPercent->value());
        cg.writeEntry("autoFontPercent", m_autoFontPercent);
        if (m_autoFont) {
            m_font.setPointSize(fontSize());
        }
    }

    //TODO
    if (m_useThemeColor != ui.useThemeColor->isChecked()) {
        changed = true;
        m_useThemeColor = ui.useThemeColor->isChecked();
        cg.writeEntry("useThemeColor", m_useThemeColor);
    }

    QColor newColor = ui.textColorButton->color();
    if (m_textColor != newColor) {
        changed = true;
        m_textColor = newColor;
        cg.writeEntry("textcolor", m_textColor);
	QTextCursor textCursor = m_textEdit->nativeWidget()->textCursor();
	m_textEdit->nativeWidget()->selectAll();
        m_textEdit->nativeWidget()->setTextColor(m_textColor);
	m_textEdit->nativeWidget()->setTextCursor(textCursor);
    }

    bool spellCheck = ui.checkSpelling->isChecked();
    if (spellCheck != m_checkSpelling) {
        changed = true;
        m_checkSpelling = spellCheck;
        cg.writeEntry("checkSpelling", m_checkSpelling);
        m_textEdit->nativeWidget()->setCheckSpellingEnabled(m_checkSpelling);
    }

    QString tmpColor = m_colorActions[ui.notesColorComboBox->currentIndex()]->property("color").toString();
    if (tmpColor != m_color){
        m_color = tmpColor;
        cg.writeEntry("color", tmpColor);
        changed = true;
    }

    if (changed) {
        updateTextGeometry();
        emit configNeedsSaving();
    }
}

#include "notes.moc"
