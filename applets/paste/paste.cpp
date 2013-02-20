/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "paste.h"
#include "sendkeys.h"
#include "list.h"
#include "snippetconfig.h"
#include "autopasteconfig.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QTimer>

#include <KIcon>
#include <KIconLoader>
#include <KColorScheme>
#include <KConfigDialog>

#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/Theme>

Paste::Paste(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_list(0), m_snippetConfig(0)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(200, 300);
    setPopupIcon("edit-paste");
}

Paste::~Paste()
{
    m_list->deleteLater();
    m_list = 0;
}

void Paste::init()
{
    cfg = globalConfig();

    m_list = new ListForm;
    connect(&cfg, SIGNAL(changed(ConfigData)), m_list, SLOT(setData(ConfigData)));
    connect(m_list, SIGNAL(textCopied()), this, SLOT(showOk()));
    m_list->setData(cfg);
}

void Paste::createConfigurationInterface(KConfigDialog *parent)
{
    m_snippetConfig = new SnippetConfig;
    connect(&cfg, SIGNAL(changed(ConfigData)),
            m_snippetConfig, SLOT(setData(ConfigData)));
    m_snippetConfig->setData(cfg);
    m_autoPasteConfig = new AutoPasteConfig;
    connect(&cfg, SIGNAL(changed(ConfigData)),
            m_autoPasteConfig, SLOT(setData(ConfigData)));
    m_autoPasteConfig->setData(cfg);

    parent->addPage(m_snippetConfig, i18n("Texts"), "accessories-text-editor");
    parent->addPage(m_autoPasteConfig, i18n("Automatic Paste"), "edit-paste");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(m_snippetConfig->textEdit, SIGNAL(textChanged()), parent, SLOT(settingsModified()));
    connect(m_snippetConfig->nameEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(m_snippetConfig->list, SIGNAL(itemSelectionChanged()), parent, SLOT(settingsModified()));
    connect(m_snippetConfig->addMacroButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_snippetConfig->addButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_snippetConfig->removeButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_autoPasteConfig->autoPasteCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified())); 
    connect(m_autoPasteConfig->addButton, SIGNAL(clicked()), parent, SLOT(settingsModified())); 
    connect(m_autoPasteConfig->editButton, SIGNAL(clicked()), parent, SLOT(settingsModified())); 
    connect(m_autoPasteConfig->removeButton, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(m_autoPasteConfig->pasteKeyButton, SIGNAL(keySequenceChanged(QKeySequence)), parent, SLOT(settingsModified()));
    connect(m_autoPasteConfig->appsTreeView, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
}

void Paste::configAccepted()
{
    m_snippetConfig->getData(&cfg);
    m_autoPasteConfig->getData(&cfg);
    m_list->setData(cfg);
    cfg.writeEntries();
    emit configNeedsSaving();
}

void Paste::configChanged()
{
    cfg.readEntries();
}

void Paste::showOk()
{
    // Show ok icon like in DeviceNotifier
    setPopupIcon("dialog-ok");
    m_list->icon->setPixmap(KIcon("dialog-ok").pixmap(KIconLoader::SizeSmall,
                                                      KIconLoader::SizeSmall));
    QTimer::singleShot(2000, this, SLOT(resetIcon()));
}

void Paste::resetIcon()
{
    setPopupIcon("edit-paste");
    m_list->icon->setPixmap(KIcon("edit-paste").pixmap(KIconLoader::SizeSmall,
                                                       KIconLoader::SizeSmall));
}

QWidget *Paste::widget()
{
    return m_list;
}

#include "paste.moc"
