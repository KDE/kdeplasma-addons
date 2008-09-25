/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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
#include <KDebug>
#include <KIcon>
#include <KMimeType>
#include <KUrl>
#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <QDataStream>
#include <plasma/panelsvg.h>

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

class PopupGraphicsView : public QGraphicsView {
public:
    PopupGraphicsView(
        QGraphicsScene * scene,
        Lancelot::ActionListView * list,
        QWidget * parent
    ) : QGraphicsView(scene, parent), m_list(list)
    {
        setWindowFlags(Qt::FramelessWindowHint);
        setFrameStyle(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        setCacheMode(QGraphicsView::CacheBackground);

        m_bg.setImagePath("dialogs/background");
        m_bg.setEnabledBorders(Plasma::PanelSvg::NoBorder);
    }

    void drawBackground(QPainter * painter, const QRectF & rect)
    {
        Q_UNUSED(rect);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
        m_bg.resizePanel(rect.size());
        m_bg.paintPanel(painter);
    }

    virtual void setVisible(bool visible)
    {
        if (visible) {
            m_list->setPreferredSize(QSizeF(size()));
            m_list->setMinimumSize(QSizeF(size()));
            m_list->setGeometry(QRectF(
                        QPointF(),
                        QSizeF(size())
                   ));
        }
        QGraphicsView::setVisible(visible);
    }

private:
    Lancelot::ActionListView * m_list;
    Plasma::PanelSvg m_bg;

};

class FullLayout: public QGraphicsLayout {
public:
    FullLayout(QGraphicsLayoutItem * parent = 0)
        : QGraphicsLayout(parent)
    {
    }

    virtual int count() const
    {
        return m_items.size();
    }

    void setGeometry(const QRectF & rect)
    {
        QGraphicsLayout::setGeometry(rect);
        invalidate();
    }

    virtual void invalidate()
    {
        foreach (QGraphicsLayoutItem * item, m_items) {
            item->setGeometry(geometry());
            item->setPreferredSize(geometry().size());
        }
    }

    void removeItem(QGraphicsLayoutItem * item)
    {
        m_items.removeAll(item);
    }

    virtual QGraphicsLayoutItem * itemAt(int i) const
    {
        if (i >= 0 && i < m_items.size()) {
            return m_items.at(i);
        }
        return NULL;
    }

    virtual void removeAt(int i)
    {
        if (i >= 0 && i < m_items.size()) {
            return m_items.removeAt(i);
        }
    }

    void addItem(QGraphicsLayoutItem * item)
    {
        m_items.append(item);
    }

    virtual QSizeF sizeHint(Qt::SizeHint hint, const QSizeF & constraint) const
    {
        QSizeF size;
        foreach (QGraphicsLayoutItem * item, m_items) {
            size.expandedTo(item->effectiveSizeHint(hint, constraint));
        }
        return size;
    }
private:
    QList< QGraphicsLayoutItem * > m_items;
};

LancelotPart::LancelotPart(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args), m_instance(NULL), m_list(NULL),
    m_model(NULL), m_icon(NULL), m_dialog(NULL), m_widget(NULL),
    m_scene(NULL), m_wasConstrained(false)
{
    if (args.size() > 0) {
        m_cmdarg = args[0].toString();
    }

    setAcceptDrops(true);
    setHasConfigurationInterface(true);
}

void LancelotPart::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
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
    kDebug() << data;
    return load(Serializator::serialize(data));
}

bool LancelotPart::loadFromList(const QStringList & list)
{
    bool loaded = false;

    foreach (QString line, list) {
        if (load(line)) {
            loaded = true;
        }
    }

    return loaded;
}

void LancelotPart::init()
{
    // Setting up UI
    m_instance = new Lancelot::Instance();
    m_layout = new FullLayout(this);

    m_list = new Lancelot::ActionListView();
    m_model = new Models::PartsMergedModel();
    m_list->setModel(m_model);

    setLayout(m_layout);
    setupAppletUi(true);

    m_instance->activateAll();

    connect(
            m_model, SIGNAL(removeModelRequested(int)),
            this, SLOT(removeModel(int))
           );

    // Loading data
    bool loaded = loadConfig();
    kDebug() << "loaded from config " << loaded;
    if (!loaded && (m_dialog == NULL)) {
        kDebug() << "resizing to the default size";
        resize(200, 300);
    }

    if (!loaded && !m_cmdarg.isEmpty()) {
        if (QFileInfo(QUrl(m_cmdarg).toLocalFile()).isDir()) {
            loadDirectory(m_cmdarg);
        } else {
            loadFromFile(m_cmdarg);
        }
    }

    resize(size());
    m_layout->activate();
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
            Lancelot::ActionListViewModel * model = NULL;

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
                m_model->addModel(modelID,
                        QIcon(),
                        modelID,
                        model = new Models::FolderModel(modelID));
                m_models.append(model);
            }
            loaded = (model != NULL);
        }
    }

    if (loaded) {
        if (!m_data.isEmpty()) {
            m_data += "\n";
        }
        m_data += input;
        saveConfig();
    }

    return loaded;
}

LancelotPart::~LancelotPart()
{
    if (m_dialog != NULL) {
        ((FullLayout *)m_layout)->removeItem(m_icon);
        delete m_dialog;
        delete m_icon;
    }

    ((FullLayout *)m_layout)->removeItem(m_list);
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
    KConfigGroup kcg = config();

    QString data = kcg.readEntry("partData", QString());
    if (data.isEmpty()) {
        return false;
    }
    return loadFromList(data.split("\n"));
}

void LancelotPart::removeModel(int index)
{
    Lancelot::ActionListViewModel * model = m_model->modelAt(index);
    m_model->removeModel(index);
    if (m_models.contains(model)) {
        delete model;
    }

    QStringList configs = m_data.split("\n");
    configs.removeAt(index);
    m_data = configs.join("\n");

    saveConfig();
}

void LancelotPart::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setupAppletUi();
    }

    m_layout->updateGeometry();
}

void LancelotPart::setupAppletUi(bool force)
{
    bool isSizeConstrained = (
            (formFactor() != Plasma::Planar) &&
            (formFactor() != Plasma::MediaCenter)
            );

    if (!force && (m_wasConstrained == isSizeConstrained)) {
        return;
    }
    m_wasConstrained = isSizeConstrained;

    if (isSizeConstrained) {
        ((FullLayout *) m_layout)->removeItem(m_list);
        m_list->setParentItem(NULL);

        if (m_dialog == NULL) {
            m_dialog = new Plasma::Dialog();
            m_dialog->setFocusPolicy(Qt::NoFocus);
            m_dialog->setWindowFlags(Qt::Popup);
            connect(
                    Models::ApplicationConnector::instance(), SIGNAL(doHide(bool)),
                    m_dialog, SLOT(hide())
                );

            QVBoxLayout * layout = new QVBoxLayout(m_dialog);
            layout->setSpacing(0);
            layout->setMargin(0);

            m_scene = new QGraphicsScene(0, 0, 200, 300, m_dialog);

            m_widget = new PopupGraphicsView(m_scene, m_list, m_dialog);
            layout->addWidget(m_widget);

            m_icon = new Lancelot::HoverIcon(KIcon("lancelot-part"), QString());
            connect(
                    m_icon, SIGNAL(activated()),
                    this, SLOT(iconActivated())
                   );

            m_dialog->setLayout(layout);
            m_dialog->adjustSize();

            m_dialog->installEventFilter(this);
            m_widget->installEventFilter(this);
        }

        m_scene->addItem(m_list);

        ((FullLayout *) m_layout)->addItem(m_icon);
        m_icon->setParentItem(this);

        setAspectRatioMode(Plasma::ConstrainedSquare);

        setMinimumSize(QSizeF());
    } else {
        ((FullLayout *) m_layout)->removeItem(m_icon);

        if (m_dialog != NULL) {
            delete m_dialog;
            delete m_icon;
            m_dialog = NULL;
            m_icon = NULL;
        }
        setMinimumSize(QSizeF(150, 200));
        ((FullLayout *) m_layout)->addItem(m_list);
        m_list->setParentItem(this);
        setAspectRatioMode(Plasma::IgnoreAspectRatio);
    }

    applyConfig();
}

void LancelotPart::iconActivated()
{
    m_dialog->move(popupPosition(m_dialog->sizeHint()));
    m_dialog->show();
}

bool LancelotPart::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            hidePopup();
            return true;
        }
    }
    return Plasma::Applet::eventFilter(object, event);
}

void LancelotPart::hidePopup() {
    if (m_dialog != NULL) {
        m_dialog->hide();
    }
}

void LancelotPart::createConfigurationInterface(KConfigDialog * parent)
{
    QWidget * widget = new QWidget();
    m_config.setupUi(widget);
    m_config.panelIcon->setEnabled(m_dialog != NULL);

    KConfigGroup kcg = config();
    m_config.setIcon(
            kcg.readEntry("iconLocation", "lancelot-part"));
    m_config.setIconClickActivation(
            kcg.readEntry("iconClickActivation", true));
    m_config.setContentsClickActivation(
            kcg.readEntry("contentsClickActivation", m_dialog == NULL));
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

    if (m_icon != NULL) {
        m_icon->setActivationMethod(
                kcg.readEntry("iconClickActivation", true) ?
                (Lancelot::ClickActivate) : (Lancelot::HoverActivate)
            );
        m_icon->setIcon(kcg.readEntry("iconLocation", "lancelot-part"));
    }

    if (!kcg.readEntry("contentsClickActivation", m_dialog == NULL)) {
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

#include "LancelotPart.moc"
