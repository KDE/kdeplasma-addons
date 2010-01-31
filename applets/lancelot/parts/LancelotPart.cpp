/*
 *   Copyright (C) 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <QDataStream>

#include <KDebug>
#include <KIcon>
#include <KMimeType>
#include <KUrl>
#include <KLineEdit>

#include <Plasma/FrameSvg>
#include <Plasma/Corona>
#include <Plasma/IconWidget>

#define ACTIVATION_TIME 300

LancelotPart::LancelotPart(QObject * parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_list(NULL), m_model(NULL), m_runnnerModel(NULL),
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
    m_root = new QGraphicsWidget(this);

    // m_layout = new Lancelot::FullBorderLayout();
    m_layout = new QGraphicsLinearLayout();
    m_layout->setOrientation(Qt::Vertical);

    m_root->setLayout(m_layout);

    m_searchText = new Plasma::LineEdit(m_root);
    m_searchText->nativeWidget()->setClearButtonShown(true);
    m_searchText->nativeWidget()->setClickMessage(i18nc("Enter the text to search for", "Search..."));
    connect(m_searchText->widget(),
        SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

    m_list = new Lancelot::ActionListView(m_root);
    m_list->setShowsExtendersOutside(false);

    m_model = new Models::PartsMergedModel();
    m_list->setModel(m_model);

    m_root->setMinimumSize(300, 200);
    m_root->setPreferredSize(300, 400);

    m_list->setMinimumWidth(300);

    m_layout->addItem(m_searchText);
    m_layout->addItem(m_list);
    m_layout->setStretchFactor(m_list, 2);

    connect(
            m_model, SIGNAL(removeModelRequested(int)),
            this, SLOT(removeModel(int))
    );

    // Listening to immutability
    Plasma::Corona * corona = (Plasma::Corona *) scene();
    immutabilityChanged(corona->immutability());
    connect(corona, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
    immutabilityChanged(Plasma::Mutable);

    // Loading data
    bool loaded = loadConfig();

    if (!loaded && !m_cmdarg.isEmpty()) {
        KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(m_cmdarg));
        m_model->append(m_cmdarg, fileItem);
        saveConfig();
    }

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
    event->setAccepted(
        m_model->append(event->mimeData()));
    saveConfig();
}

bool LancelotPart::loadFromList(const QStringList & list)
{
    bool loaded = false;

    foreach (const QString& line, list) {
        kDebug() << line;
        if (m_model->load(line)) {
            loaded = true;
        }
    }

    return loaded;
}

LancelotPart::~LancelotPart()
{
    delete m_model;
}

void LancelotPart::saveConfig()
{
    KConfigGroup kcg = config();
    kcg.writeEntry("partData", m_model->serializedData());
    kcg.sync();
}

bool LancelotPart::loadConfig()
{
    applyConfig();

    KConfigGroup kcg = config();

    QString data = kcg.readEntry("partData", QString());

    kDebug() << data;

    if (data.isEmpty()) {
        return false;
    }
    return loadFromList(data.split('\n'));
}

void LancelotPart::removeModel(int index)
{
    m_model->remove(index);
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
        if (event->type() == QEvent::GraphicsSceneHoverEnter) {
            m_timer.start(ACTIVATION_TIME, this);
        } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
            m_timer.stop();
        }
    }

    if (event->type() == QEvent::KeyPress) {
        bool pass = false;
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Tab:
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_list->selectedIndex() == -1) {
                    m_list->initialSelection();
                }
            default:
                pass = true;
        }

        if (pass) {
            m_list->keyPressEvent(keyEvent);
        }

        m_searchText->nativeWidget()->setFocus();
        m_searchText->setFocus();
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
    m_config.setShowSearchBox(
            kcg.readEntry("showSearchBox", false));

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

    showSearchBox(kcg.readEntry("showSearchBox", false));
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
    kcg.writeEntry("showSearchBox", m_config.showSearchBox());

    kcg.sync();
    applyConfig();
}

void LancelotPart::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    PopupApplet::resizeEvent(event);
}

QGraphicsWidget * LancelotPart::graphicsWidget()
{
    return m_root;
}

void LancelotPart::immutabilityChanged(Plasma::ImmutabilityType value)
{
    kDebug() << value;
    Lancelot::Global::self()->setImmutability(value);
}

void LancelotPart::search(const QString & query)
{
    if (!m_runnnerModel) {
        m_runnnerModel = new Lancelot::Models::Runner(true);
    }

    if (query.isEmpty()) {
        m_list->setModel(m_model);
    } else {
        m_runnnerModel->setSearchString(query);
        m_list->setModel(m_runnnerModel);
    }
}

void LancelotPart::showSearchBox(bool value)
{
    if (m_searchText->isVisible() == value) {
        return;
    }

    m_searchText->setVisible(value);

    if (value) {
        m_layout->insertItem(0, m_searchText);
    } else {
        m_layout->removeItem(m_searchText);
    }
}

#include "LancelotPart.moc"
