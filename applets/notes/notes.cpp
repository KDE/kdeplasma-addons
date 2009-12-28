/***************************************************************************
 *   Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net>         *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2008 Davide Bettio <davide.bettio@kdemail.net>          *
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

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QScrollBar>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QParallelAnimationGroup>

#include <KConfigDialog>
#include <KConfigGroup>
#include <KGlobalSettings>
#include <KFileDialog>
#include <KMessageBox>
#include <KIcon>
#include <KPushButton>
#include <KStandardAction>
#include <KAction>

#include <Plasma/Animator>
#include <plasma/animation.h>
#include <Plasma/PushButton>
#include <Plasma/Theme>

NotesTextEdit::NotesTextEdit(QWidget *parent)
    : KRichTextEdit(parent),
      m_formatMenu(0)
{
}

NotesTextEdit::~NotesTextEdit()
{
}

void NotesTextEdit::setFormatMenu(QMenu *menu)
{
    m_formatMenu = menu;
}

/**
 * Customize the context menu
 */
void NotesTextEdit::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *popup = mousePopupMenu();

    popup->addSeparator();
    popup->addAction(KStandardAction::saveAs(this, SLOT(saveToFile()), this));

    if (m_formatMenu) {
        popup->addMenu(m_formatMenu);
    }

    popup->exec(event->pos());
    delete popup;
}

void NotesTextEdit::italic()
{
    setTextItalic(!fontItalic());
}

void NotesTextEdit::bold()
{
    setTextBold(fontWeight() != 75);
}

void NotesTextEdit::underline()
{
    setTextUnderline(!fontUnderline());
}

void NotesTextEdit::strikeOut()
{
    QFont a = currentFont();
    setTextStrikeOut(!a.strikeOut());
}

void NotesTextEdit::justifyCenter()
{
    if (alignment() == Qt::AlignHCenter) {
        setAlignment(Qt::AlignLeft);
    } else {
        alignCenter();
    }
}

void NotesTextEdit::justifyFill()
{
    if (alignment() == Qt::AlignJustify) {
        setAlignment(Qt::AlignLeft);
    } else {
        alignJustify();
    }
}

/**
 * Add to mousePressEvent a signal to change the edited line's background color
 */
void NotesTextEdit::mousePressEvent ( QMouseEvent * event )
{
    KTextEdit::mousePressEvent(event);
    if(event->button()== Qt::LeftButton)
      emit cursorMoved();
}

/**
 * Same as mousePressEvent
 */
void NotesTextEdit::keyPressEvent ( QKeyEvent * event )
{
  KTextEdit::keyPressEvent(event);
  switch(event->key())
  {
    case Qt::Key_Left : emit cursorMoved(); break;
    case Qt::Key_Right : emit cursorMoved(); break;
    case Qt::Key_Down : emit cursorMoved(); break;
    case Qt::Key_Up : emit cursorMoved(); break;
    default:break;
  }
}

/**
* Scale text on wheel scrolling with control pressed
*/
void NotesTextEdit::wheelEvent ( QWheelEvent * event )
{
    if (event->modifiers() & Qt::ControlModifier ) {
        if (event->orientation() == Qt::Horizontal) {
            return;
        } else {
            if (event->delta() > 0) {
                emit scrolledUp();
            } else {
                emit scrolledDown();
            }
        }
        event->accept();
    } else {
        KTextEdit::wheelEvent(event);
    }
}

/**
 * Add to the Note a signal to prevent from leaving the note and remove line background color
 * when there is no focus on the plasmoid
 */
void NotesTextEdit::leaveEvent ( QEvent * event )
{
  KTextEdit::leaveEvent(event);
  emit mouseUnhovered();
}

/**
 * Use notesTextEdit as native widget instead of KTextEdit
 */
PlasmaTextEdit::PlasmaTextEdit(QGraphicsWidget *parent)
    : Plasma::TextEdit(parent)
{
    KTextEdit *w = nativeWidget();
    native = new NotesTextEdit;
    native->setWindowFlags(native->windowFlags() | Qt::BypassGraphicsProxyWidget);
    //FIXME: we need a way to just add actions without changing the native widget under its feet
    if (native->verticalScrollBar() && w->verticalScrollBar()) {
        native->verticalScrollBar()->setStyle(w->verticalScrollBar()->style());
    }
    setNativeWidget(native);

    connect(native, SIGNAL(cursorMoved()), this, SIGNAL(textChanged()));
    connect(native, SIGNAL(mouseUnhovered()), this, SIGNAL(mouseUnhovered()));

    // scrollwheel + ctrl changes font size
    connect(native, SIGNAL(scrolledUp()), parent, SLOT(increaseFontSize()));
    connect(native, SIGNAL(scrolledDown()), parent, SLOT(decreaseFontSize()));
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

/**
* remove the background color of the last line edited when leaving
*/
void Notes::mouseUnhovered()
{
    QTextCursor textCursor = m_textEdit->nativeWidget()->textCursor();
    QTextEdit::ExtraSelection textxtra;
    textxtra.cursor = m_textEdit->nativeWidget()->textCursor();
    textxtra.cursor.movePosition( QTextCursor::StartOfLine );
    textxtra.cursor.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor );
    textxtra.format.setBackground( Qt::transparent );

    QList<QTextEdit::ExtraSelection> extras;
    extras << textxtra;
    m_textEdit->nativeWidget()->setExtraSelections( extras );

    update();
}

Notes::Notes(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_wheelFontAdjustment(0),
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
    m_textEdit->setMinimumSize(QSize(60, 60)); //Ensure a minimum size (height) for the textEdit
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->setSpacing(2); //We need a bit of spacing between the edit and the buttons
    m_textEdit->nativeWidget()->setFrameShape(QFrame::NoFrame);
    m_textEdit->nativeWidget()->viewport()->setAutoFillBackground(false);
    m_layout->addItem(m_textEdit);

    if (args.count() > 0) {
        KUrl url = KUrl(args.at(0).toString());
        QFile f(url.path());

        if (f.open(QIODevice::ReadOnly)) {
            QTextStream t(&f);
            m_textEdit->nativeWidget()->setHtml(t.readAll());
            f.close();
        }
    }

    createTextFormatingWidgets();
    setLayout(m_layout);
}

Notes::~Notes()
{
    saveNote();
    delete m_textEdit;
    delete m_colorMenu;
    delete m_formatMenu;
}

void Notes::init()
{
    m_notesTheme.setImagePath("widgets/notes");
    m_notesTheme.setContainsMultipleImages(false);

    m_colorMenu = new QMenu(i18n("Notes Color"));
    connect(m_colorMenu, SIGNAL(triggered(QAction*)), this, SLOT(changeColor(QAction*)));
    addColor("white", i18n("White"));
    addColor("black", i18n("Black"));
    addColor("red", i18n("Red"));
    addColor("orange", i18n("Orange"));
    addColor("yellow", i18n("Yellow"));
    addColor("green", i18n("Green"));
    addColor("blue", i18n("Blue"));
    addColor("pink", i18n("Pink"));
    addColor("translucent", i18n("Translucent"));

    m_autoFont = false;

    KConfigGroup cg = config();
    m_color = cg.readEntry("color", "yellow");
    // color must be before setPlainText("foo")
    m_useThemeColor = cg.readEntry("useThemeColor", true);
    m_useNoColor = cg.readEntry("useNoColor", true);
    if (m_useThemeColor) {
        m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    } else {
        m_textColor = cg.readEntry("textColor", m_textColor);
    }
    m_textBackgroundColor = cg.readEntry("textBackgroundColor", QColor(Qt::transparent));
    m_textEdit->nativeWidget()->setTextColor(m_textColor);

    QString text = cg.readEntry("autoSaveHtml", QString());
    if (text.isEmpty()) {
        // see if the old, plain text version is still there?
        text = cg.readEntry("autoSave", QString());
        if (!text.isEmpty()) {
            m_textEdit->nativeWidget()->setText(text);
            cg.deleteEntry("autoSave");
            saveNote();
        }
    } else {
        m_textEdit->setText(text);
    }

    int scrollValue = cg.readEntry("scrollValue").toInt();
    if (scrollValue) {
        m_textEdit->nativeWidget()->verticalScrollBar()->setValue(scrollValue);
    }

    m_font = cg.readEntry("font", KGlobalSettings::generalFont());
    //Set the font family on init, it may have changed from the outside
    QTextCursor oldCursor = m_textEdit->nativeWidget()->textCursor();
    m_textEdit->nativeWidget()->selectAll();
    m_textEdit->nativeWidget()->setFontFamily(m_font.family());
    m_textEdit->nativeWidget()->setTextCursor(oldCursor);

    m_customFontSize = cg.readEntry("customFontSize", m_font.pointSize());
    m_autoFont = cg.readEntry("autoFont", true);
    m_autoFontPercent = cg.readEntry("autoFontPercent", 4);

    m_checkSpelling = cg.readEntry("checkSpelling", false);
    m_textEdit->nativeWidget()->setCheckSpellingEnabled(m_checkSpelling);
    updateTextGeometry();

    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(delayedSaveNote()));
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(lineChanged()));
    connect(m_textEdit, SIGNAL(mouseUnhovered()), this, SLOT(mouseUnhovered()));
}

/**
* this function is called when you change the line you are editing
* to change the background color
*/
void Notes::lineChanged()
{
    //Re-set the formatting if previous text was deleted
    if (m_textEdit->nativeWidget()->document()->characterCount() == 1) {
        QTextCharFormat fmt;
        fmt.setForeground(QBrush(m_textColor));
        fmt.setFont(m_font);
        m_textEdit->nativeWidget()->setCurrentCharFormat(fmt);
    }

    if (m_useNoColor) {
        return;
    }

    QTextCursor textCursor = m_textEdit->nativeWidget()->textCursor();
    QTextEdit::ExtraSelection textxtra;
    textxtra.cursor = m_textEdit->nativeWidget()->textCursor();
    textxtra.cursor.movePosition( QTextCursor::StartOfLine );
    textxtra.cursor.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor );
    textxtra.format.setBackground( m_textBackgroundColor );

    QList<QTextEdit::ExtraSelection> extras;
    extras << textxtra;
    m_textEdit->nativeWidget()->setExtraSelections( extras );

    update();
}

void Notes::constraintsEvent(Plasma::Constraints constraints)
{
    //XXX why does everything break so horribly if I remove this line?
    setBackgroundHints(Plasma::Applet::NoBackground);
    if (constraints & Plasma::SizeConstraint) {
        updateTextGeometry();
    }

    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
            setAspectRatioMode(Plasma::ConstrainedSquare);
        } else {
            setAspectRatioMode(Plasma::IgnoreAspectRatio);
        }
    }
}

void Notes::updateTextGeometry()
{
    if (m_layout) {
        //FIXME: this needs to come from the svg
        const qreal xpad = geometry().width() / 15;
        const qreal ypad = geometry().height() / 15;
        m_layout->setContentsMargins(xpad, ypad, xpad, ypad);
        m_font.setPointSize(fontSize());

        QTextCursor oldTextCursor = m_textEdit->nativeWidget()->textCursor();
        m_textEdit->nativeWidget()->selectAll();
        m_textEdit->nativeWidget()->setFontPointSize(m_font.pointSize());
        m_textEdit->nativeWidget()->setTextCursor(oldTextCursor);

        if (m_autoFont) {
            lineChanged();
        }
    }
}

int Notes::fontSize()
{
    if (m_autoFont) {
        int autosize = qRound(((geometry().width() + geometry().height())/2)*m_autoFontPercent/100)  + m_wheelFontAdjustment;
        return qMax(KGlobalSettings::smallestReadableFont().pointSize(), autosize);
    } else {
        return m_customFontSize + m_wheelFontAdjustment;
    }
}

void Notes::increaseFontSize()
{
    m_wheelFontAdjustment++;
    updateTextGeometry();
}

void Notes::decreaseFontSize()
{

    if (KGlobalSettings::smallestReadableFont().pointSize() < fontSize()) {
        m_wheelFontAdjustment--;
        updateTextGeometry();
    }
}

void Notes::delayedSaveNote()
{
    m_saveTimer.start(5000);
}

void Notes::saveNote()
{
    KConfigGroup cg = config();
    cg.writeEntry("autoSaveHtml", m_textEdit->text());
    cg.writeEntry("scrollValue", QVariant(m_textEdit->nativeWidget()->verticalScrollBar()->value()));
    //kDebug() << m_textEdit->nativeWidget()->toPlainText();
    emit configNeedsSaving();
}

void Notes::themeChanged()
{
    m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    update();
}

void Notes::addColor(const QString &id, const QString &colorName)
{
    if (m_notesTheme.hasElement(id + "-notes")) {
        QAction *tmpAction = m_colorMenu->addAction(colorName);
        tmpAction->setProperty("color", id);
    }
}

void Notes::changeColor(QAction *action)
{
    if (!action || action->property("color").type() != QVariant::String) {
        return;
    }

    m_color = action->property("color").toString();
    KConfigGroup cg = config();
    cg.writeEntry("color", m_color);
    emit configNeedsSaving();
    update();
}

QList<QAction *> Notes::contextualActions()
{
    QList<QAction *> actions;
    actions.append(m_colorMenu->menuAction());
    actions.append(m_formatMenu->menuAction());
    return actions;
}

void Notes::paintInterface(QPainter *p,
                           const QStyleOptionGraphicsItem *option,
                           const QRect &contentsRect)
{
    Q_UNUSED(option);

    m_notesTheme.resize(geometry().size());
    if (m_notesTheme.hasElement(m_color + "-notes")) {
        m_notesTheme.paint(p, contentsRect, m_color + "-notes");
    } else {
        m_notesTheme.paint(p, contentsRect, "yellow-notes");
    }
}

void Notes::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), "knotes");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QButtonGroup *fontSizeGroup = new QButtonGroup(widget);
    fontSizeGroup->addButton(ui.autoFont);
    fontSizeGroup->addButton(ui.customFont);

    ui.textColorButton->setColor(m_textColor);
    ui.textBackgroundColorButton->setColor(m_textBackgroundColor);
    ui.fontStyleComboBox->setCurrentFont(m_font);
    ui.fontBoldCheckBox->setChecked(m_font.bold());
    ui.fontItalicCheckBox->setChecked(m_font.italic());
    ui.autoFont->setChecked(m_autoFont);
    ui.autoFontPercent->setEnabled(m_autoFont);
    ui.customFont->setChecked(!m_autoFont);
    ui.customFontSizeSpinBox->setEnabled(!m_autoFont);
    ui.autoFontPercent->setValue(m_autoFontPercent);
    ui.customFontSizeSpinBox->setValue(m_customFontSize);

    QButtonGroup *FontColorGroup = new QButtonGroup(widget);
    FontColorGroup->addButton(ui.useThemeColor);
    FontColorGroup->addButton(ui.useCustomColor);
    ui.useThemeColor->setChecked(m_useThemeColor);
    ui.useCustomColor->setChecked(!m_useThemeColor);

    QButtonGroup *BackgroundColorGroup = new QButtonGroup(widget);
    BackgroundColorGroup->addButton(ui.useNoColor);
    BackgroundColorGroup->addButton(ui.useCustomBackgroundColor);
    ui.useNoColor->setChecked(m_useNoColor);
    ui.useCustomBackgroundColor->setChecked(!m_useNoColor);

    ui.checkSpelling->setChecked(m_checkSpelling);

    QList<QAction *> colorActions = m_colorMenu->actions();
    for (int i = 0; i < colorActions.size(); i++){
        QString text = colorActions.at(i)->text();
        if (!text.isEmpty()){
            ui.notesColorComboBox->insertItem(i, text);
            if (colorActions.at(i)->property("color").toString() == m_color) {
                ui.notesColorComboBox->setCurrentIndex(i);
            }
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

    //Apply bold and italic changes (if any) here (this is destructive formatting)
    bool boldChanged = (m_font.bold() != newFont.bold());
    bool italicChanged = (m_font.italic() != newFont.italic());
    if (boldChanged || italicChanged) {
        //Save previous selection
        QTextCursor oldCursor = m_textEdit->nativeWidget()->textCursor();
        m_textEdit->nativeWidget()->selectAll();
        if (boldChanged) {
            m_textEdit->nativeWidget()->setFontWeight(newFont.weight());
        }
        if (italicChanged) {
            m_textEdit->nativeWidget()->setFontItalic(newFont.italic());
        }
        //Restore previous selection
        m_textEdit->nativeWidget()->setTextCursor(oldCursor);
    }

    //Save font settings to config
    if (m_font != newFont) {
        changed = true;
        cg.writeEntry("font", newFont);
        m_font = newFont;

        //Apply font family
        QTextCursor oldCursor = m_textEdit->nativeWidget()->textCursor();
        m_textEdit->nativeWidget()->selectAll();
        m_textEdit->nativeWidget()->setFontFamily(m_font.family());
        m_textEdit->nativeWidget()->setTextCursor(oldCursor);
    }

    if (m_customFontSize != ui.customFontSizeSpinBox->value()) {
        changed = true;
        m_customFontSize = ui.customFontSizeSpinBox->value();
        cg.writeEntry("customFontSize", m_customFontSize);
    }

    if (m_autoFont != ui.autoFont->isChecked()) {
        changed = true;
        m_autoFont = ui.autoFont->isChecked();
        cg.writeEntry("autoFont", m_autoFont);
    }

    if (m_autoFontPercent != ui.autoFontPercent->value()) {
        changed = true;
        m_autoFontPercent = (ui.autoFontPercent->value());
        cg.writeEntry("autoFontPercent", m_autoFontPercent);
    }

    disconnect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    bool textColorChanged = false;
    if (m_useThemeColor != ui.useThemeColor->isChecked()) {
        changed = true;
        textColorChanged = true;
        m_useThemeColor = ui.useThemeColor->isChecked();
        cg.writeEntry("useThemeColor", m_useThemeColor);
        m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    }

    if (!m_useThemeColor) {
        QColor newColor = ui.textColorButton->color();
        if (m_textColor != newColor) {
            changed = true;
            textColorChanged = true;
            m_textColor = newColor;
            cg.writeEntry("textColor", m_textColor);
        }
    }

    if (textColorChanged) {
        QTextCursor textCursor = m_textEdit->nativeWidget()->textCursor();
        m_textEdit->nativeWidget()->selectAll();
        m_textEdit->nativeWidget()->setTextColor(m_textColor);
        m_textEdit->nativeWidget()->setTextCursor(textCursor);
    }

    if (m_useNoColor != ui.useNoColor->isChecked()) {
        changed = true;
        m_useNoColor = ui.useNoColor->isChecked();
        cg.writeEntry("useNoColor", m_useNoColor);
        QTextCursor textCursor = m_textEdit->nativeWidget()->textCursor();
        QTextEdit::ExtraSelection textxtra;
        textxtra.cursor = m_textEdit->nativeWidget()->textCursor();
        textxtra.cursor.movePosition( QTextCursor::StartOfLine );
        textxtra.cursor.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor );
        textxtra.format.setBackground( Qt::transparent );

        QList<QTextEdit::ExtraSelection> extras;
        extras << textxtra;
        m_textEdit->nativeWidget()->setExtraSelections( extras );
    }

    QColor newBackgroundColor = ui.textBackgroundColorButton->color();
    if (m_textBackgroundColor != newBackgroundColor) {
        changed = true;
        m_textBackgroundColor = newBackgroundColor;
        cg.writeEntry("textBackgroundColor", m_textBackgroundColor);
    }

    bool spellCheck = ui.checkSpelling->isChecked();
    if (spellCheck != m_checkSpelling) {
      changed = true;
        m_checkSpelling = spellCheck;
        cg.writeEntry("checkSpelling", m_checkSpelling);
        m_textEdit->nativeWidget()->setCheckSpellingEnabled(m_checkSpelling);
    }

    QList<QAction *> colorActions = m_colorMenu->actions();
    QString tmpColor = colorActions[ui.notesColorComboBox->currentIndex()]->property("color").toString();
    if (tmpColor != m_color){
        m_color = tmpColor;
        cg.writeEntry("color", m_color);
        changed = true;
    }

    if (changed) {
        updateTextGeometry();
        update();
        emit configNeedsSaving();
    }
}

void Notes::createTextFormatingWidgets()
{
    m_formatMenu = new QMenu(i18n("Formatting"));
    m_textEdit->native->setFormatMenu(m_formatMenu);
    QAction *actionBold = m_formatMenu->addAction(KIcon("format-text-bold"), i18n("Bold"));
    QAction *actionItalic = m_formatMenu->addAction(KIcon("format-text-italic"),i18n("Italic"));
    QAction *actionUnderline = m_formatMenu->addAction(KIcon("format-text-underline"), i18n("Underline"));
    QAction *actionStrikeThrough = m_formatMenu->addAction(KIcon("format-text-strikethrough"), i18n("StrikeOut"));
    QAction *actionCenter = m_formatMenu->addAction(KIcon("format-justify-center"), i18n("Justify center"));
    QAction *actionFill = m_formatMenu->addAction(KIcon("format-justify-fill"), i18n("Justify"));
    connect(actionItalic, SIGNAL(triggered()), m_textEdit->native, SLOT(italic()));
    connect(actionBold, SIGNAL(triggered()), m_textEdit->native, SLOT(bold()));
    connect(actionUnderline, SIGNAL(triggered()), m_textEdit->native, SLOT(underline()));
    connect(actionStrikeThrough, SIGNAL(triggered()), m_textEdit->native, SLOT(strikeOut()));
    connect(actionCenter, SIGNAL(triggered()), m_textEdit->native, SLOT(justifyCenter()));
    connect(actionFill, SIGNAL(triggered()), m_textEdit->native, SLOT(justifyFill()));

    QGraphicsWidget *widget = new QGraphicsWidget(this);
    widget->setMaximumHeight(25);

    QGraphicsLinearLayout *buttonLayout = new QGraphicsLinearLayout(Qt::Horizontal, widget);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(buttonLayout);

    m_buttonOption = new Plasma::ToolButton(widget);
    m_buttonOption->nativeWidget()->setIcon(KIcon("draw-text"));
    m_buttonOption->nativeWidget()->setCheckable(true);

    m_buttonBold = new Plasma::ToolButton(widget);
    m_buttonBold->setAction(actionBold);

    m_buttonItalic = new Plasma::ToolButton(widget);
    m_buttonItalic->setAction(actionItalic);

    m_buttonUnderline = new Plasma::ToolButton(widget);
    m_buttonUnderline->setAction(actionUnderline);

    m_buttonStrikeThrough = new Plasma::ToolButton(widget);
    m_buttonStrikeThrough->setAction(actionStrikeThrough);

    m_buttonCenter = new Plasma::ToolButton(widget);
    m_buttonCenter->setAction(actionCenter);

    m_buttonFill = new Plasma::ToolButton(widget);
    m_buttonFill->setAction(actionFill);

    buttonLayout->addItem(m_buttonOption);
    buttonLayout->addStretch();
    buttonLayout->addItem(m_buttonBold);
    buttonLayout->addItem(m_buttonItalic);
    buttonLayout->addItem(m_buttonUnderline);
    buttonLayout->addItem(m_buttonStrikeThrough);
    buttonLayout->addItem(m_buttonCenter);
    buttonLayout->addItem(m_buttonFill);
    buttonLayout->addStretch();

    m_layout->addItem(widget);

    m_buttonAnimGroup = new QParallelAnimationGroup(this);

    for (int i = 0; i < 6; i++){
        m_buttonAnim[i] = Plasma::Animator::create(Plasma::Animator::FadeAnimation, this);
        m_buttonAnimGroup->addAnimation(m_buttonAnim[i]);
    }

    m_buttonAnim[0]->setWidgetToAnimate(m_buttonBold);
    m_buttonAnim[1]->setWidgetToAnimate(m_buttonItalic);
    m_buttonAnim[2]->setWidgetToAnimate(m_buttonUnderline);
    m_buttonAnim[3]->setWidgetToAnimate(m_buttonStrikeThrough);
    m_buttonAnim[4]->setWidgetToAnimate(m_buttonCenter);
    m_buttonAnim[5]->setWidgetToAnimate(m_buttonFill);

    showOptions(false);
    connect(m_buttonOption->nativeWidget(), SIGNAL(toggled(bool)), this, SLOT(showOptions(bool)));
}

void Notes::showOptions(bool show)
{
    m_buttonOption->nativeWidget()->setDown(show);

    qreal targetOpacity = show ? 1 : 0;
    qreal startOpacity = 1 - targetOpacity;

    for (int i = 0; i < 6; i++){
        m_buttonAnim[i]->setProperty("startOpacity", startOpacity);
        m_buttonAnim[i]->setProperty("targetOpacity", targetOpacity);
    }

    m_buttonAnimGroup->start();
}

#include "notes.moc"
