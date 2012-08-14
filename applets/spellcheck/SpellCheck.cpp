/***********************************************************************************
* Spell Check: Plasmoid for fast spell checking.
* Copyright (C) 2008 - 2010 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#include "SpellCheck.h"

#include <QFile>
#include <QClipboard>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextStream>
#include <QApplication>
#include <QGraphicsLinearLayout>

#include <KIcon>
#include <KAction>
#include <KLocale>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>

K_EXPORT_PLASMA_APPLET(spellcheck, SpellCheck)

SpellCheck::SpellCheck(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
       m_textEdit(NULL),
       m_spellingDialog(NULL),
       m_dictionaryComboBox(NULL),
       m_dragTimer(0)
{
    KGlobal::locale()->insertCatalog("spellcheck");
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setAcceptDrops(true);

    const int iconSize = KIconLoader::SizeSmallMedium;
    resize(iconSize, iconSize);
}

SpellCheck::~SpellCheck()
{
    delete m_spellingDialog;
}

void SpellCheck::init()
{
    Plasma::IconWidget *icon = new Plasma::IconWidget(KIcon("tools-check-spelling"), QString(), this);

    registerAsDragHandle(icon);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(icon);

    Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent(i18n("Spell Checking"), i18n("Check spelling of clipboard contents."), icon->icon().pixmap(IconSize(KIconLoader::Desktop))));

    connect(this, SIGNAL(activate()), this, SLOT(toggleDialog()));
    connect(icon, SIGNAL(clicked()), this, SLOT(toggleDialog()));
}

void SpellCheck::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!m_spellingDialog || !m_spellingDialog->isVisible()) {
        m_dragTimer = startTimer(500);
    }

    Plasma::Applet::dragEnterEvent(event);
}

void SpellCheck::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (m_dragTimer) {
        killTimer(m_dragTimer);
    }

    Plasma::Applet::dragLeaveEvent(event);
}

void SpellCheck::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (m_dragTimer) {
        killTimer(m_dragTimer);
    }

    if (!KUrl::List::canDecode(event->mimeData())) {
        if (event->mimeData()->hasText()) {
            if (!m_spellingDialog || !m_spellingDialog->isVisible()) {
                toggleDialog(false);
            }

            m_textEdit->setText(event->mimeData()->text());

            event->accept();
        } else {
            event->ignore();
        }

        return;
    }

    if (!m_spellingDialog || !m_spellingDialog->isVisible()) {
        toggleDialog(false);

        m_textEdit->clear();
    }

    KUrl::List droppedUrls = KUrl::List::fromMimeData(event->mimeData());
    QTextCursor cursor = m_textEdit->textCursor();

    for (int i = 0; i < droppedUrls.count(); ++i) {
        QFile file(droppedUrls.at(i).toLocalFile());

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream contents(file.readAll());
            contents.setAutoDetectUnicode(true);

            cursor.insertText(contents.readAll());
        }

        file.close();
    }

    event->accept();
}

void SpellCheck::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Applet::mousePressEvent(event);

    if (event->button() == Qt::MidButton) {
        event->accept();
    }
}

void SpellCheck::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Applet::mouseReleaseEvent(event);

    if (event->button() == Qt::MidButton) {
        toggleDialog(true, false);
    }
}

void SpellCheck::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_dragTimer) {
        if (!m_spellingDialog || !m_spellingDialog->isVisible()) {
            toggleDialog(false);
        }
    }

    killTimer(event->timerId());
}

void SpellCheck::configChanged()
{
    if (m_spellingDialog) {
        m_spellingDialog->resize(config().readEntry("dialogSize", m_spellingDialog->size()));
    }

    if (m_textEdit) {
        setLanguage(config().readEntry("dictionary", m_textEdit->highlighter()->currentLanguage()));
    }
}

void SpellCheck::toggleDialog(bool pasteText, bool preferSelection)
{
    if (!m_spellingDialog) {
        m_spellingDialog = new Plasma::Dialog();
        KWindowSystem::setState(m_spellingDialog->effectiveWinId(), NET::SkipTaskbar|NET::SkipPager);
        m_spellingDialog->setFocusPolicy(Qt::NoFocus);
        m_spellingDialog->setWindowTitle(i18n("Spell checking"));
        m_spellingDialog->setWindowIcon(KIcon("tools-check-spelling"));
        m_spellingDialog->setResizeHandleCorners(Plasma::Dialog::All);

        m_textEdit = new KTextEdit(m_spellingDialog);
        m_textEdit->enableFindReplace(false);
        m_textEdit->setCheckSpellingEnabled(true);
        m_textEdit->createHighlighter();

        m_dictionaryComboBox = new Sonnet::DictionaryComboBox(m_spellingDialog);
        m_dictionaryComboBox->setToolTip(i18n("Language"));

        KAction *spellingAction = new KAction(KIcon("tools-check-spelling"), i18n("Spell checking"), m_spellingDialog);
        KAction *copyAction = new KAction(KIcon("edit-copy"), i18n("Copy"), m_spellingDialog);
        KAction *closeAction = new KAction(KIcon("dialog-close"), i18n("Close"), m_spellingDialog);

        QToolButton *spellingButton = new QToolButton(m_spellingDialog);
        spellingButton->setDefaultAction(spellingAction);

        QToolButton *copyButton = new QToolButton(m_spellingDialog);
        copyButton->setDefaultAction(copyAction);

        QToolButton *closeButton = new QToolButton(m_spellingDialog);
        closeButton->setDefaultAction(closeAction);

        QHBoxLayout *horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(m_dictionaryComboBox);
        horizontalLayout->addWidget(spellingButton);
        horizontalLayout->addWidget(copyButton);
        horizontalLayout->addWidget(closeButton);

        QVBoxLayout *verticalLayout = new QVBoxLayout(m_spellingDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setMargin(0);
        verticalLayout->addWidget(m_textEdit);
        verticalLayout->addLayout(horizontalLayout);

        configChanged();

        connect(m_spellingDialog, SIGNAL(dialogResized()), this, SLOT(dialogResized()));
        connect(spellingAction, SIGNAL(triggered()), m_textEdit, SLOT(checkSpelling()));
        connect(copyAction, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
        connect(closeAction, SIGNAL(triggered()), this, SLOT(toggleDialog()));
        connect(m_textEdit, SIGNAL(languageChanged(QString)), this, SLOT(setLanguage(QString)));
        connect(m_dictionaryComboBox, SIGNAL(dictionaryChanged(QString)), this, SLOT(setLanguage(QString)));
    }

    if (m_spellingDialog->isVisible()) {
        m_spellingDialog->animatedHide(Plasma::locationToInverseDirection(location()));

        m_textEdit->clear();
    } else {
        m_spellingDialog->move(popupPosition(m_spellingDialog->sizeHint()));
        m_spellingDialog->animatedShow(Plasma::locationToDirection(location()));

        if (pasteText) {
            m_textEdit->setText((!preferSelection || QApplication::clipboard()->text(QClipboard::Selection).isEmpty()) ? QApplication::clipboard()->text(QClipboard::Clipboard) : QApplication::clipboard()->text(QClipboard::Selection));
        }

        m_textEdit->setFocus();
    }
}

void SpellCheck::dialogResized()
{
    if (!m_spellingDialog) {
        return;
    }

    config().writeEntry("dialogSize", m_spellingDialog->size());

    emit configNeedsSaving();
}

void SpellCheck::copyToClipboard()
{
    if (!m_textEdit) {
        return;
    }

    if (!m_textEdit->textCursor().selectedText().isEmpty()) {
        QApplication::clipboard()->setText(m_textEdit->textCursor().selectedText());
    } else {
        QApplication::clipboard()->setText(m_textEdit->toPlainText());
    }
}

void SpellCheck::setLanguage(const QString &language)
{
    if (language != m_textEdit->spellCheckingLanguage()) {
        m_textEdit->setSpellCheckingLanguage(language);
    }

    m_dictionaryComboBox->setCurrentByDictionary(language);

    config().writeEntry("dictionary", language);

    emit configNeedsSaving();
}

#include "SpellCheck.moc"
