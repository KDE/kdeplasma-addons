/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotPart.h"
#include <KIcon>
#include <KMimeType>
#include <KUrl>
#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <QDataStream>
#include <plasma/framesvg.h>
#include <plasma/widgets/iconwidget.h>

#include "../models/BaseModel.h"
#include "../models/Devices.h"
#include "../models/Places.h"
#include "../models/SystemServices.h"
#include "../models/RecentDocuments.h"
#include "../models/OpenDocuments.h"
#include "../models/NewDocuments.h"
#include "../models/FolderModel.h"
#include "../models/FavoriteApplications.h"
#include "../models/Applications.h"
#include "../models/Runner.h"
#include "../models/ContactsKopete.h"
#include "../models/MessagesKmail.h"
#include "../Serializator.h"

#define ACTIVATION_TIME 300

LancelotPart::LancelotPart(QObject * parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_instance(NULL), m_list(NULL), m_model(NULL),
    m_icon(NULL)
{
    if (args.size() > 0) {
        m_cmdarg = args[0].toString();
    }

    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setPopupIcon("lancelot-part");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    // setPassivePopup(true);

    foreach (QGraphicsItem * child, childItems()) {
        Plasma::IconWidget * icon = dynamic_cast < Plasma::IconWidget * > (child);
        if (icon) {
            m_icon = icon;
            m_icon->installEventFilter(this);
        }
    }

}

void LancelotPart::init()
{
    // Setting up UI
    m_instance = new Lancelot::Instance();
    m_list = new Lancelot::ActionListView(this);
    m_model = new Models::PartsMergedModel();
    m_list->setModel(m_model);
    m_instance->activateAll();

    connect(
            m_model, SIGNAL(removeModelRequested(int)),
            this, SLOT(removeModel(int))
    );

    // Loading data
    bool loaded = loadConfig();

    if (!loaded && !m_cmdarg.isEmpty()) {
        KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(m_cmdarg));
        if (fileItem.mimetype() == "inode/directory") {
            loadDirectory(m_cmdarg);
        } else {
            loadFromFile(m_cmdarg);
        }
    }

    m_list->setMinimumSize(150, 200);
    m_list->setPreferredSize(250, 300);
    KGlobal::locale()->insertCatalog("lancelot");
    applyConfig();
}

void LancelotPart::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if (event->mimeData()->hasFormat("text/x-lancelotpart")) {
        event->setAccepted(true);
        return;
    }

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        event->setAccepted(false);
        return;
    }

    QString file = event->mimeData()->data("text/uri-list");
    KMimeType::Ptr mimeptr = KMimeType::findByUrl(KUrl(file));
    if (!mimeptr) {
        event->setAccepted(false);
        return;
    }
    QString mime = mimeptr->name();
    event->setAccepted(mime == "text/x-lancelotpart" || mime == "inode/directory");
}

void LancelotPart::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    if (event->mimeData()->hasFormat("text/x-lancelotpart")) {
        event->setAccepted(true);
        QString data = event->mimeData()->data("text/x-lancelotpart");
        load(data);
        return;
    }

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        event->setAccepted(false);
        return;
    }

    QString file = event->mimeData()->data("text/uri-list");
    KMimeType::Ptr mimeptr = KMimeType::findByUrl(KUrl(file));
    if (!mimeptr) {
        event->setAccepted(false);
        return;
    }
    QString mime = mimeptr->name();
    event->setAccepted(mime == "text/x-lancelotpart" || mime == "inode/directory");

    if (mime == "inode/directory") {
        loadDirectory(file);
    } else {
        loadFromFile(file);
    }
}

bool LancelotPart::loadFromFile(const QString & url)
{
    bool loaded = false;
    QFile file(QUrl(url).toLocalFile());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (load(line)) {
                loaded = true;
            }
        }
    }

    return loaded;
}

bool LancelotPart::loadDirectory(const QString & url)
{
    QMap < QString, QString > data;
    data["version"]     = "1.0";
    data["type"]        = "list";
    data["model"]       = "Folder " + url;
    return load(Serializator::serialize(data));
}

bool LancelotPart::loadFromList(const QStringList & list)
{
    bool loaded = false;

    foreach (const QString& line, list) {
        if (load(line)) {
            loaded = true;
        }
    }

    return loaded;
}

bool LancelotPart::load(const QString & input)
{
    QMap < QString, QString > data = Serializator::deserialize(input);

    if (!data.contains("version")) {
        return false;
    }

    bool loaded = false;

    if (data["version"] <= "1.0") {
        if (data["type"] == "list") {
            QString modelID = data["model"];
            Lancelot::ActionListModel * model = NULL;

            if (modelID == "Places") {
                m_model->addModel(modelID, QIcon(), i18n("Places"), model = new Models::Places());
                m_models.append(model);
            } else if (modelID == "System") {
                m_model->addModel(modelID, QIcon(), i18n("System"), model = new Models::SystemServices());
                m_models.append(model);
            } else if (modelID == "Devices/Removable") {
                m_model->addModel(modelID, QIcon(), i18n("Removable devices"), model = new Models::Devices(Models::Devices::Removable));
                m_models.append(model);
            } else if (modelID == "Devices/Fixed") {
                m_model->addModel(modelID, QIcon(), i18n("Fixed devices"), model = new Models::Devices(Models::Devices::Fixed));
                m_models.append(model);
            } else if (modelID == "NewDocuments") {
                m_model->addModel(modelID, QIcon(), i18n("New Documents"), model = new Models::NewDocuments());
                m_models.append(model);
            } else if (modelID == "OpenDocuments") {
                m_model->addModel(modelID, QIcon(), i18n("Open Documents"), model = new Models::OpenDocuments());
                m_models.append(model);
            } else if (modelID =="RecentDocuments") {
                m_model->addModel(modelID, QIcon(), i18n("Recent Documents"), model = new Models::RecentDocuments());
                m_models.append(model);
            } else if (modelID =="Messages") {
                m_model->addModel(modelID, QIcon(), i18n("Unread messages"), model = new Models::MessagesKmail());
                m_models.append(model);
            } else if (modelID =="Contacts") {
                m_model->addModel(modelID, QIcon(), i18n("Online contacts"), model = new Models::ContactsKopete());
                m_models.append(model);
            } else if (modelID == "FavoriteApplications") {
                // We don't want to delete this one (singleton)
                m_model->addModel(modelID, QIcon(), i18n("Favorite Applications"), model = Models::FavoriteApplications::instance());
            } else if (modelID.startsWith("Folder ")) {
                modelID.remove(0, 7);
                if (modelID.startsWith("applications:/")) {
                    modelID.remove(0, 14);
                    m_model->addModel(modelID,
                        QIcon(),
                        modelID,
                        model = new Models::Applications(modelID, QString(), QIcon(), true));
                } else {
                    m_model->addModel(modelID,
                        QIcon(),
                        modelID,
                        model = new Models::FolderModel(modelID));
                }
                m_models.append(model);
            }
            loaded = (model != NULL);
        }
    }

    if (loaded) {
        if (!m_data.isEmpty()) {
            m_data += '\n';
        }
        m_data += input;
        saveConfig();
    }

    return loaded;
}

LancelotPart::~LancelotPart()
{
    qDeleteAll(m_models);
    delete m_model;
    delete m_instance;
}

void LancelotPart::saveConfig()
{
    KConfigGroup kcg = config();
    kcg.writeEntry("partData", m_data);
    kcg.sync();
}

bool LancelotPart::loadConfig()
{
    applyConfig();

    KConfigGroup kcg = config();

    QString data = kcg.readEntry("partData", QString());
    if (data.isEmpty()) {
        return false;
    }
    return loadFromList(data.split('\n'));
}

void LancelotPart::removeModel(int index)
{
    Lancelot::ActionListModel * model = m_model->modelAt(index);
    m_model->removeModel(index);
    if (m_models.contains(model)) {
        delete model;
        m_models.removeAll(model);
    }

    QStringList configs = m_data.split('\n');
    configs.removeAt(index);
    m_data = configs.join("\n");

    saveConfig();
}

void LancelotPart::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        showPopup();
    }
    PopupApplet::timerEvent(event);
}

bool LancelotPart::eventFilter(QObject * object, QEvent * event)
{
    if (!m_iconClickActivation && object == m_icon) {
        if (event->type() == QEvent::QEvent::GraphicsSceneHoverEnter) {
            m_timer.start(ACTIVATION_TIME, this);
        } else if (event->type() == QEvent::QEvent::GraphicsSceneHoverLeave) {
            m_timer.stop();
        }
    }

    return Plasma::PopupApplet::eventFilter(object, event);
}

void LancelotPart::createConfigurationInterface(KConfigDialog * parent)
{
    QWidget * widget = new QWidget();
    m_config.setupUi(widget);
    m_config.panelIcon->setVisible(m_list->parentItem() == NULL);

    KConfigGroup kcg = config();

    QString iconPath = kcg.readEntry("iconLocation", "lancelot-part");
    m_config.setIcon(iconPath);
    if (iconPath == "lancelot-part") {
        m_config.setIcon(popupIcon());
    }

    m_config.setIconClickActivation(
            kcg.readEntry("iconClickActivation", true));
    m_config.setContentsClickActivation(
            kcg.readEntry("contentsClickActivation", m_list->parentItem() == NULL));
    m_config.setContentsExtenderPosition(
            (Lancelot::ExtenderPosition)
            kcg.readEntry("contentsExtenderPosition",
            (int)Lancelot::RightExtender));

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), icon());
}

void LancelotPart::applyConfig()
{
    KConfigGroup kcg = config();

    QString icon = kcg.readEntry("iconLocation", "lancelot-part");
    setPopupIcon(icon);

    if (icon == "lancelot-part") {
        if (m_model->modelCount() > 0) {
            Lancelot::ActionListModel * model = m_model->modelAt(0);
            if (!model->selfIcon().isNull()) {
                setPopupIcon(model->selfIcon());
            }
        }
    }
    m_iconClickActivation = kcg.readEntry("iconClickActivation", true);

    if (!kcg.readEntry("contentsClickActivation", m_list->parentItem() == NULL)) {
       m_list->setExtenderPosition(
               (Lancelot::ExtenderPosition)
               kcg.readEntry("contentsExtenderPosition",
               (int)Lancelot::RightExtender));
    } else {
        m_list->setExtenderPosition(Lancelot::NoExtender);
    }
}

void LancelotPart::configAccepted()
{
    KConfigGroup kcg = config();

    kcg.writeEntry("iconLocation",
            m_config.icon());
    kcg.writeEntry("iconClickActivation",
            m_config.iconClickActivation());
    kcg.writeEntry("contentsClickActivation",
            m_config.contentsClickActivation());
    kcg.writeEntry("contentsExtenderPosition",
            (int)m_config.contentsExtenderPosition());

    kcg.sync();
    applyConfig();
}

void LancelotPart::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    PopupApplet::resizeEvent(event);
}

QGraphicsWidget * LancelotPart::graphicsWidget()
{
    return m_list;
}

#include "LancelotPart.moc"
