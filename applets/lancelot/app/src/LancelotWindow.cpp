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

#include "LancelotWindow.h"
#include <kwindowsystem.h>

#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QDesktopWidget>

#include <KRecentDocument>
#include <KActionCollection>
#include <KStandardAction>
#include <KAuthorized>
#include <KGlobalAccel>
#include <KShortcutsDialog>
#include <KConfigDialog>
#include <KProcess>
#include <plasma/animator.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <kworkspace/kworkspace.h>
// #include "ksmserver_interface.h"
// #include "screensaver_interface.h"
#include <Serializator.h>

#include "models/Devices.h"
#include "models/Places.h"
#include "models/SystemServices.h"
#include "models/RecentDocuments.h"
#include "models/OpenDocuments.h"
#include "models/NewDocuments.h"
#include "models/FolderModel.h"
#include "models/FavoriteApplications.h"
#include "models/Applications.h"
#include "models/Runner.h"
#include "models/SystemActions.h"
#include "models/ContactsKopete.h"
#include "models/MessagesKmail.h"

#include "models/BaseMergedModel.h"
#include "logger/Logger.h"

#include <lancelot/widgets/ResizeBordersPanel.h>
#include <lancelot/widgets/PopupMenu.h>
#include <lancelot/models/PlasmaServiceListModel.h>

#include <KLineEdit>
#include <Plasma/LineEdit>
#include <KAboutApplicationDialog>
#include <KCmdLineArgs>

#define sectionsWidth 128
#define windowHeightDefault 500
#define mainWidthDefault    422

#define HIDE_TIMER_INTERVAL 1500
#define SEARCH_TIMER_INTERVAL 300

LancelotWindow::LancelotWindow()
    : m_root(NULL), m_corona(NULL),
    m_hovered(false), m_showingFull(true), m_sectionsSignalMapper(NULL),
    m_config("lancelotrc"), m_mainConfig(&m_config, "Main"),
    instance(NULL),
    m_configWidget(NULL),
    m_resizeDirection(None),
    m_mainSize(mainWidthDefault, windowHeightDefault),
    m_skipEvent(false),
    menuSystemButton(NULL),
    menuLancelotContext(NULL)
{
    setFocusPolicy(Qt::WheelFocus);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky);

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hideImmediate()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    m_hideTimer.setSingleShot(true);

    m_corona = new Plasma::Corona(this);

    setMinimumSize(400, 300);
    setScene(m_corona);

    instance = new Lancelot::Instance();
    instance->setHasApplication(true);

    m_root = new Lancelot::ResizeBordersPanel();
    installEventFilter(this);

    m_root->setGroupByName("RootPanel");
    m_corona->addItem(m_root);

    setupUi(m_root);
    m_root->setLayoutItem(layoutMain);

    setupModels();

    /* TODO: Convert this to PUCK generated code */
    editSearch = new Plasma::LineEdit();
    editSearch->setParentItem(m_root);
    editSearch->nativeWidget()->setClearButtonShown(true);
    editSearch->nativeWidget()->setClickMessage(i18n("Search"));
    editSearch->show();
    layoutSearch->addItem(editSearch,
        Lancelot::NodeLayout::NodeCoordinate(0.0, 0.5, 0, 0),
        Lancelot::NodeLayout::NodeCoordinate(1.0, 0.5, 0, INFINITY)
    );
    editSearch->nativeWidget()->installEventFilter(this);
    editSearch->setFocusPolicy(Qt::WheelFocus);
    editSearch->nativeWidget()->setFocusPolicy(Qt::WheelFocus);

    passagewayApplications->setEntranceTitle(i18n("Favorites"));
    passagewayApplications->setEntranceIcon(KIcon("favorites"));
    passagewayApplications->setAtlasTitle(i18n("Applications"));
    passagewayApplications->setAtlasIcon(KIcon("applications-other"));
    /* End TODO */

    // instance->activateAll();

    m_sectionsSignalMapper = new QSignalMapper(this);
    connect (m_sectionsSignalMapper,
        SIGNAL(mapped(const QString &)),
        this,
        SLOT(sectionActivated(const QString &))
    );

    QMapIterator<QString, Lancelot::ExtenderButton * > i(sectionButtons);
    while (i.hasNext()) {
        i.next();
        connect(i.value(), SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
        m_sectionsSignalMapper->setMapping(i.value(), i.key());
    }

    connect(buttonSystem1, SIGNAL(activated()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem2, SIGNAL(activated()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem3, SIGNAL(activated()), this, SLOT(systemButtonClicked()));

    connect(buttonLancelotContext,  SIGNAL(activated()), this, SLOT(lancelotContext()));

    connect(editSearch->widget(),
        SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

    instance->activateAll();
    loadConfig();
    setupActions();

    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget *, QWidget *)),
            this, SLOT(focusChanged(QWidget *, QWidget *)));
}

void LancelotWindow::drawBackground(QPainter * painter, const QRectF & rect)
{
    painter->setCompositionMode(QPainter::CompositionMode_Clear);
    painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
    painter->setCompositionMode(QPainter::CompositionMode_Source);
}

void LancelotWindow::focusChanged(QWidget * old, QWidget * now)
{
    Q_UNUSED(old);

    // We have to return the focus to the QGraphicsView
    // since it handles the keyboard, the universe
    // and everything
    if (now == this) {
        editSearch->nativeWidget()->setFocus();
        editSearch->setFocus();
    }
}

LancelotWindow::~LancelotWindow()
{
    delete m_configWidget;
    delete instance;
}

void LancelotWindow::lancelotShow(int x, int y)
{
    m_showingFull = true;
    showWindow(x, y);
}

void LancelotWindow::lancelotShowCentered()
{
    m_showingFull = true;
    showWindow(0, 0, true);
}

void LancelotWindow::lancelotShowItem(int x, int y, const QString & name)
{
    sectionActivated(name);
    m_showingFull = false;
    showWindow(x, y);
}

void LancelotWindow::lancelotHide(bool immediate)
{
    if (immediate) {
        editSearch->setText(QString());
        if (m_configUi.appbrowserReset()) {
            passagewayApplications->reset();
        }
        hide();
        return;
    }

    if (m_hovered) return;
    m_hideTimer.start();
}

void LancelotWindow::showWindow(int x, int y, bool centered)
{
    panelSections->setVisible(m_showingFull);

    layoutMain->setSize((m_showingFull ? sectionsWidth : 0), Lancelot::FullBorderLayout::LeftBorder);

    m_resizeDirection = None;
    m_hideTimer.stop();

    if (isVisible()) {
        // We are exiting because we do not want to move already opened window
        // because most probably it is just invoked from the same applet and
        // needs to show only another category
        resizeWindow();
        return;
    }

    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(x, y));
    resizeWindow();

    Plasma::Flip flip;

    if (!centered) {
        flip = Plasma::VerticalFlip;

        if (x < screenRect.left()) {
            x = screenRect.left();
        }

        if (y < screenRect.top()) {
            y = screenRect.top();
        }

        if (x + width() > screenRect.right()) {
            x = x - width();
            flip |= Plasma::HorizontalFlip;
        }

        if (y + height() > screenRect.bottom()) {
            y = y - height();
            flip &= ~Plasma::VerticalFlip;
        }
    } else {
        flip = Plasma::NoFlip;

        x = screenRect.left()
            + (screenRect.width() - width()) / 2;
        y = screenRect.top()
            + (screenRect.height() - height()) / 2;
    }

    layoutMain->setFlip(flip);
    layoutSections->setFlip(flip);

    if (m_configUi.activationMethod() == LancelotConfig::NoClick) {
        instance->group("SystemButtons")->setProperty("ExtenderPosition", QVariant(
                (flip & Plasma::VerticalFlip)?(Lancelot::TopExtender):(Lancelot::BottomExtender)
        ));
        instance->group("SystemButtons")
            ->setProperty("ActivationMethod", Lancelot::ExtenderActivate);
    } else {
        instance->group("SystemButtons")
            ->setProperty("ExtenderPosition", QVariant(Lancelot::NoExtender));
        instance->group("SystemButtons")
            ->setProperty("ActivationMethod", Lancelot::ClickActivate);
    }
    instance->group("LancelotContext")->setProperty("ExtenderPosition",
            instance->group("SystemButtons")->property("ExtenderPosition"));
    instance->group("LancelotContext")->setProperty("ActivationMethod",
            instance->group("SystemButtons")->property("ActivationMethod"));
    instance->group("SystemButtons")->notifyUpdated();
    instance->group("LancelotContext")->notifyUpdated();

    if (m_showingFull) {
        sectionActivated("applications");
    }

    move(x, y);
    show();
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    KWindowSystem::forceActiveWindow(winId());

    //editSearch->clearFocus();
    editSearch->nativeWidget()->setFocus();
    editSearch->setFocus();
}

void LancelotWindow::resizeWindow()
{
    QSize newSize = m_mainSize;
    if (m_showingFull) {
        newSize.rwidth() += sectionsWidth;
    }
    resize(newSize);
    m_root->group()->backgroundSvg()->resizeFrame(newSize);
    setMask(m_root->group()->backgroundSvg()->mask());
}

QStringList LancelotWindow::sectionIDs()
{
    QStringList res;
    res << "applications" << "computer" << "contacts" << "documents";
    return res;
}

QStringList LancelotWindow::sectionNames()
{
    QStringList res;
    res << i18n("Applications") << i18n("Computer") << i18n("Contacts") << i18n("Documents");
    return res;
}

QStringList LancelotWindow::sectionIcons()
{
    QStringList res;
    res << "applications-other" << "computer-laptop" << "kontact" << "applications-office";
    return res;
}

void LancelotWindow::sectionActivated(const QString & item)
{
    if (item == m_activeSection) {
        return;
    }

    m_activeSection = item;

    foreach (Lancelot::ExtenderButton * button, sectionButtons) {
        button->setChecked(false);
    }

    if (sectionButtons.contains(item)) {
        sectionButtons[item]->setChecked(true);
    }

    if (m_focusIndex >= 0 && m_focusIndex < m_focusList.count()) {
        if (m_focusList.at(m_focusIndex) != passagewayApplications) {
            ((Lancelot::ActionListView *) m_focusList.at(m_focusIndex))->clearSelection();
        } else {
            ((Lancelot::PassagewayView *) m_focusList.at(m_focusIndex))->clearSelection();
        }
    }

    m_focusList.clear();
    m_focusIndex = 0;

    if (item == "search") {
        m_focusList << listSearchLeft;
    } else if (item == "applications") {
        m_focusList << passagewayApplications;
    } else if (item == "computer") {
        m_focusList << listComputerLeft;
        m_focusList << listComputerRight;
    } else if (item == "contacts") {
        m_focusList << listContactsLeft;
        m_focusList << listContactsRight;
    } else if (item == "documents") {
        m_focusList << listDocumentsLeft;
        m_focusList << listDocumentsRight;
    }

    layoutCenter->show(item);
}

void LancelotWindow::search(const QString & string)
{
    if (editSearch->text() != string) {
        editSearch->setText(string);
    }

    m_searchString = string;

    ((Models::Runner *) m_models["Runner"])->setSearchString(m_searchString);
    if (m_searchString.isEmpty()) {
        sectionActivated("applications");
    } else {
        sectionActivated("search");
    }
}

void LancelotWindow::systemButtonClicked()
{
    Lancelot::ExtenderButton * button =
        static_cast < Lancelot::ExtenderButton * > (sender());

    Lancelot::ActionTreeModel * model =
        Models::SystemActions::instance()->action(systemButtonActions[button]);

    if (!model) return;

    if (!menuSystemButton) {
        menuSystemButton = new Lancelot::PopupList();
        menuSystemButton->resize(200, 200);
        Models::ApplicationConnector * ac = Models::ApplicationConnector::instance();
        connect(
                ac, SIGNAL(doHide(bool)),
                menuSystemButton, SLOT(close())
        );
    }

    menuSystemButton->setModel(model);
    menuSystemButton->updateSize();

    QRect g = button->geometry().toRect();
    g.moveTopLeft(g.topLeft() + geometry().topLeft());

    if (layoutMain->flip() & Plasma::VerticalFlip) {
        menuSystemButton->move(g.bottomLeft());
    } else {
        menuSystemButton->move(g.topLeft() - QPoint(0, menuSystemButton->geometry().height()));
    }

    menuSystemButton->show();
}

void LancelotWindow::setupModels()
{
    // Models:
    m_models["Places"]            = new Models::Places();
    m_models["SystemServices"]    = new Models::SystemServices();
    m_models["Devices/Removable"] = new Models::Devices(Models::Devices::Removable);
    m_models["Devices/Fixed"]     = new Models::Devices(Models::Devices::Fixed);

    m_models["NewDocuments"]      = new Models::NewDocuments();
    m_models["RecentDocuments"]   = new Models::RecentDocuments();
    m_models["OpenDocuments"]     = new Models::OpenDocuments();

    // m_models["Contacts"]          = new Models::ContactsKopete();
    // m_models["Messages"]          = new Models::MessagesKmail();

    m_models["Runner"]            = new Models::Runner();

    // Groups:

    m_modelGroups["ComputerLeft"]   = new Models::BaseMergedModel();
    m_modelGroups["DocumentsLeft"]  = new Models::BaseMergedModel();
    m_modelGroups["ContactsLeft"]   = new Models::BaseMergedModel();

    m_modelGroups["ComputerRight"]  = new Models::BaseMergedModel();
    m_modelGroups["DocumentsRight"] = new Models::BaseMergedModel();
    m_modelGroups["ContactsRight"]  = new Models::BaseMergedModel();

    // Assignments: Model - Group:
    #define MergedAddModel(MergedModel, ModelID, Model) \
        ((Models::BaseMergedModel *)(MergedModel))      \
        ->addModel((ModelID), QIcon(), (Model)->selfTitle(), Model);

    MergedAddModel(m_modelGroups["ComputerLeft"], "Places", m_models["Places"]);
    MergedAddModel(m_modelGroups["ComputerLeft"], "System", m_models["SystemServices"]);

    MergedAddModel(m_modelGroups["ComputerRight"], "Devices/Removable", m_models["Devices/Removable"]);
    MergedAddModel(m_modelGroups["ComputerRight"], "Devices/Fixed", m_models["Devices/Fixed"]);

    MergedAddModel(m_modelGroups["DocumentsLeft"], "NewDocuments", m_models["NewDocuments"]);

    MergedAddModel(m_modelGroups["DocumentsRight"], "OpenDocuments", m_models["OpenDocuments"]);
    MergedAddModel(m_modelGroups["DocumentsRight"], "RecentDocuments", m_models["RecentDocuments"]);

    QString plugins;

    // Contacts Mail
    plugins = m_mainConfig.readEntry("mailPlugins", QString());
    if (plugins.isEmpty()) {
        m_models["Messages"]          = new Models::MessagesKmail();
        MergedAddModel(m_modelGroups["ContactsLeft"], "Messages", m_models["Messages"]);
    } else if (plugins != "disabled") {
        Lancelot::ActionListModel * model;
        foreach (QString plugin, plugins.split(',')) {
            model = new Lancelot::PlasmaServiceListModel(plugin);
            MergedAddModel(m_modelGroups["ContactsLeft"], model->selfTitle(), model);
        }
    }

    // Contacts IM
    plugins = m_mainConfig.readEntry("imPlugins", QString());
    if (plugins.isEmpty()) {
        m_models["Contacts"]          = new Models::ContactsKopete();
        MergedAddModel(m_modelGroups["ContactsRight"], "Contacts", m_models["Contacts"]);
    } else if (plugins != "disabled") {
        Lancelot::ActionListModel * model;
        foreach (QString plugin, plugins.split(',')) {
            model = new Lancelot::PlasmaServiceListModel(plugin);
            MergedAddModel(m_modelGroups["ContactsRight"], model->selfTitle(), model);
        }
    }

    m_modelGroups["SearchLeft"] = m_models["Runner"];

    // Assignments: ListView - Group

    listComputerLeft->setModel(m_modelGroups["ComputerLeft"]);
    listDocumentsLeft->setModel(m_modelGroups["DocumentsLeft"]);
    listContactsLeft->setModel(m_modelGroups["ContactsLeft"]);
    listSearchLeft->setModel(m_modelGroups["SearchLeft"]);

    listComputerRight->setModel(m_modelGroups["ComputerRight"]);
    listDocumentsRight->setModel(m_modelGroups["DocumentsRight"]);
    listContactsRight->setModel(m_modelGroups["ContactsRight"]);
    //listSearchRight->setModel(m_modelGroups["SearchRight"]);

    // Applications passageview
    passagewayApplications->setEntranceModel(
        new Models::FavoriteApplications::PassagewayViewProxy()
    );
    passagewayApplications->setAtlasModel(new Models::Applications());

    #undef MergedAddModel
}

// Resizing:

void LancelotWindow::mousePressEvent(QMouseEvent * e)
{
    m_resizeDirection = None;

    if (e->x() >= width() - m_root->borderSize(Plasma::RightMargin))  m_resizeDirection |= Right;
    else if (e->x() <= m_root->borderSize(Plasma::LeftMargin))        m_resizeDirection |= Left;

    if (e->y() >= height() - m_root->borderSize(Plasma::BottomMargin)) m_resizeDirection |= Down;
    else if (e->y() <= m_root->borderSize(Plasma::TopMargin))          m_resizeDirection |= Up;

    if (m_resizeDirection != None) {
        m_originalMousePosition  = e->globalPos();
        m_originalWindowPosition = pos();
        m_originalMainSize       = m_mainSize;
    }

    QGraphicsView::mousePressEvent(e);
}

void LancelotWindow::mouseReleaseEvent(QMouseEvent * e)
{
    if (m_resizeDirection != None) {
        m_mainConfig.writeEntry("width",  m_mainSize.width());
        m_mainConfig.writeEntry("height", m_mainSize.height());
        m_mainConfig.sync();
        m_resizeDirection = None;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void LancelotWindow::mouseMoveEvent(QMouseEvent * e)
{
    if (m_resizeDirection != None) {
        m_mainSize = m_originalMainSize;

        QPoint newWindowPosition = m_originalWindowPosition;
        QPoint diff = e->globalPos() - m_originalMousePosition;

        if (m_resizeDirection & Right) {
            m_mainSize.rwidth() += diff.x();
        } else if (m_resizeDirection & Left) {
            m_mainSize.rwidth() -= diff.x();
            newWindowPosition.rx() += diff.x();
        }

        if (m_resizeDirection & Down) {
            m_mainSize.rheight() += diff.y();
        } else if (m_resizeDirection & Up) {
            m_mainSize.rheight() -= diff.y();
            newWindowPosition.ry() += diff.y();
        }

        QSize newSize = m_mainSize;
        if (m_showingFull) {
            newSize.rwidth() += sectionsWidth;
        }

        newSize = newSize.expandedTo(minimumSize());

        setGeometry(QRect(newWindowPosition, newSize));

        setMask(m_root->group()->backgroundSvg()->mask());
    }
    QGraphicsView::mouseMoveEvent(e);
}

void LancelotWindow::sendKeyEvent(QKeyEvent * event)
{
    // We get this twice for every keypress... ???
    m_skipEvent = !m_skipEvent;
    if (!m_skipEvent) {
        return;
    }

    if (passagewayApplications == m_focusList.at(m_focusIndex)) {
        ((Lancelot::PassagewayView *) m_focusList.at(m_focusIndex))->keyPressEvent(event);
    } else {
        ((Lancelot::ActionListView *) m_focusList.at(m_focusIndex))->keyPressEvent(event);
    }
}

bool LancelotWindow::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        bool pass = false;
        int oindex = m_focusIndex;
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Escape:
                lancelotHide(true);
            case Qt::Key_Tab:
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_activeSection == "search") {
                    if (listSearchLeft->selectedIndex() == -1) {
                        listSearchLeft->initialSelection();
                    }
                }
            case Qt::Key_Up:
            case Qt::Key_Down:
                sendKeyEvent(keyEvent);
                break;
            case Qt::Key_Left:
                m_focusIndex--;
                break;
            case Qt::Key_Right:
                m_focusIndex++;
                break;
            default:
                pass = true;
        }

        if (m_focusIndex < 0) {
            m_focusIndex = 0;
            pass = true;
        } else if (m_focusIndex >= m_focusList.size()) {
            m_focusIndex = m_focusList.size() - 1;
            pass = true;
        }

        if (oindex != m_focusIndex) {
            if (m_focusList.at(oindex) != passagewayApplications) {
                ((Lancelot::ActionListView *) m_focusList.at(oindex))->clearSelection();
            }
            if (m_focusList.at(m_focusIndex) != passagewayApplications) {
                ((Lancelot::ActionListView *) m_focusList.at(m_focusIndex))->initialSelection();
            }
        }

        if (pass) {
            sendKeyEvent(keyEvent);
        }


        editSearch->nativeWidget()->setFocus();
        editSearch->setFocus();
    }
    return QWidget::eventFilter(object, event);
}

void LancelotWindow::setupActions()
{
    m_actionCollection = new KActionCollection(this);
    KAction * a = 0;

    if (KAuthorized::authorizeKAction("show_lancelot")) {
        a = m_actionCollection->addAction(i18n("Lancelot"), this);
        a->setText(i18n("Open Lancelot menu"));
        a->setGlobalShortcut(KShortcut(Qt::ALT + Qt::Key_F5));
        a->setIcon(KIcon("lancelot"));
        connect(
                a, SIGNAL(triggered(bool)),
                this, SLOT(lancelotShowCentered())
                );
    }

    QStringList sIDs = sectionIDs();
    QStringList sNames = sectionNames();
    QStringList sIcons = sectionIcons();
    for (int i = 0; i < sIDs.size(); i++) {
        a = m_actionCollection->addAction(sIDs.at(i), this);
        a->setText(sNames.at(i));
        a->setIcon(KIcon(sIcons.at(i)));
        a->setShortcut(Qt::ALT + Qt::Key_1 + i);
        connect(
                a, SIGNAL(triggered(bool)),
                m_sectionsSignalMapper, SLOT(map())
                );
        m_sectionsSignalMapper->setMapping(a, sIDs.at(i));
    }
    m_actionCollection->readSettings();
    m_actionCollection->associateWidget(this);
}

void LancelotWindow::configureShortcuts()
{
    lancelotHide(true);
    KShortcutsDialog::configure(m_actionCollection);
}

void LancelotWindow::configurationChanged()
{
    loadConfig();
}

void LancelotWindow::loadConfig()
{
    // Non configurable options
    m_mainSize = QSize(
        m_mainConfig.readEntry("width",  mainWidthDefault),
        m_mainConfig.readEntry("height", windowHeightDefault)
    );

    // Creating the config widget
    if (m_configWidget == NULL) {
        m_configWidget = new QWidget();
        m_configUi.setupUi(m_configWidget);
    }
    m_configUi.loadConfig();

    // Loading activation method for groups
    bool sectionNoClick = true;
    bool listsNoClick = true;
    bool systemNoClick = true;

    switch (m_configUi.activationMethod()) {
        case LancelotConfig::Click:
            sectionNoClick = false;
            listsNoClick = false;
            systemNoClick = false;
            break;
        case LancelotConfig::Classic:
            listsNoClick = false;
            systemNoClick = false;
            break;
        case LancelotConfig::NoClick:
            break;
    }

    if (systemNoClick) {
        instance->group("SystemButtons")->setProperty("ExtenderPosition",
                (layoutMain->flip() & Plasma::VerticalFlip)
                    ? (Lancelot::TopExtender) : (Lancelot::BottomExtender)
        );
        instance->group("SystemButtons")
            ->setProperty("ActivationMethod", Lancelot::ExtenderActivate);
    } else {
        instance->group("SystemButtons")
            ->setProperty("ExtenderPosition", QVariant(Lancelot::NoExtender));
        instance->group("SystemButtons")
            ->setProperty("ActivationMethod", Lancelot::ClickActivate);
    }

    instance->group("LancelotContext")->setProperty("ExtenderPosition",
            instance->group("SystemButtons")->property("ExtenderPosition"));
    instance->group("LancelotContext")->setProperty("ActivationMethod",
            instance->group("SystemButtons")->property("ActivationMethod"));
    instance->group("SystemButtons")->notifyUpdated();
    instance->group("LancelotContext")->notifyUpdated();

    if (sectionNoClick) {
        instance->group("SectionButtons")
            ->setProperty("ActivationMethod", Lancelot::HoverActivate);
    } else {
        instance->group("SectionButtons")
            ->setProperty("ActivationMethod", Lancelot::ClickActivate);
    }
    instance->group("SectionButtons")->notifyUpdated();

    if (listsNoClick) {
        instance->group("ActionListView-Left")
            ->setProperty("ExtenderPosition", Lancelot::LeftExtender);
        instance->group("ActionListView-Right")
            ->setProperty("ExtenderPosition", Lancelot::RightExtender);
        instance->group("PassagewayView")
            ->setProperty("ActivationMethod", Lancelot::ExtenderActivate);
        instance->group("PopupList")
            ->setProperty("ExtenderPosition", Lancelot::RightExtender);
    } else {
        instance->group("ActionListView-Left")
            ->setProperty("ExtenderPosition", Lancelot::NoExtender);
        instance->group("ActionListView-Right")
            ->setProperty("ExtenderPosition", Lancelot::NoExtender);
        instance->group("PassagewayView")
            ->setProperty("ActivationMethod", Lancelot::ClickActivate);
        instance->group("PopupList")
            ->setProperty("ExtenderPosition", Lancelot::NoExtender);
    }
    instance->group("ActionListView-Left")->notifyUpdated();
    instance->group("ActionListView-Right")->notifyUpdated();
    instance->group("PassagewayView")->notifyUpdated();

    // PassagewayView settings
    if (m_configUi.appbrowserColumnLimitted()) {
        passagewayApplications->setColumnLimit(2);
    } else {
        passagewayApplications->setColumnLimit(22); // TODO: Temp
    }

    // Loading system buttons actions
    systemButtonActions[buttonSystem1] = m_configUi.systemButtonActions[m_configUi.buttonSystem1];
    systemButtonActions[buttonSystem2] = m_configUi.systemButtonActions[m_configUi.buttonSystem2];
    systemButtonActions[buttonSystem3] = m_configUi.systemButtonActions[m_configUi.buttonSystem3];

    QHashIterator < Lancelot::ExtenderButton *, QString > i(systemButtonActions);
    while (i.hasNext()) {
        i.next();

        i.key()->setTitle(Models::SystemActions::instance()->actionTitle(
                    i.value()));
        i.key()->setIcon(Models::SystemActions::instance()->actionIcon(
                    i.value()));
    }

    // Logging
    Logger::instance()->setEnabled(m_configUi.enableUsageStatistics());

    // Keep open
    Models::ApplicationConnector::instance()->setAutohideEnabled(!m_configUi.checkKeepOpen->isChecked());
}

void LancelotWindow::lancelotContext()
{
    if (!menuLancelotContext) {
        menuLancelotContext = new Lancelot::PopupMenu();

        connect(
                menuLancelotContext->addAction(KIcon(),
                    i18n("Menu Editor")), SIGNAL(triggered(bool)),
                this, SLOT(showMenuEditor()));

        connect(
                menuLancelotContext->addAction(KIcon("configure-shortcuts"),
                    i18n("Configure &Shortcuts...")), SIGNAL(triggered(bool)),
                this, SLOT(configureShortcuts()));

        connect(
                menuLancelotContext->addAction(KIcon("configure"),
                    i18n("Configure &Lancelot menu...")), SIGNAL(triggered(bool)),
                this, SLOT(configureMenu()));

        connect(
                menuLancelotContext->addAction(KIcon("lancelot"),
                    i18n("&About Lancelot")), SIGNAL(triggered(bool)),
                this, SLOT(showAboutDialog()));
    }

    //menuLancelotContext->show();
    menuLancelotContext->exec(QCursor::pos());
}

void LancelotWindow::configureMenu()
{
    lancelotHide(true);
    const QString dialogID = "LancelotMenuConfigurationDialog";
    KConfigDialog * dialog;

    if ((dialog = KConfigDialog::exists(dialogID))) {
        KWindowSystem::setOnDesktop(dialog->winId(), KWindowSystem::currentDesktop());
        dialog->show();
        KWindowSystem::activateWindow(dialog->winId());
        return;
    }

    KConfigSkeleton * nullManager = new KConfigSkeleton(0);
    dialog = new KConfigDialog(this, dialogID, nullManager);
    dialog->setFaceType(KPageDialog::Auto);
    dialog->setWindowTitle(i18n("Configure Lancelot menu"));
    dialog->setAttribute(Qt::WA_DeleteOnClose, false);
    dialog->addPage(m_configWidget, i18n("Configure Lancelot menu"), "lancelot");
    dialog->showButton(KDialog::Apply, false); // To follow the current Plasma applet style
    dialog->show();
    connect(dialog, SIGNAL(applyClicked()), this, SLOT( saveConfig()));
    connect(dialog, SIGNAL(okClicked()),    this, SLOT( saveConfig()));
}

void LancelotWindow::saveConfig()
{
    m_configUi.saveConfig();
    loadConfig();
}

void LancelotWindow::showAboutDialog()
{
    lancelotHide(true);

    KAboutApplicationDialog * about = new KAboutApplicationDialog(
            KCmdLineArgs::aboutData()
            );
    about->setAttribute(Qt::WA_DeleteOnClose, true);
    about->show();
}

void LancelotWindow::showMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void LancelotWindow::hideImmediate()
{
    lancelotHide(true);
}

void LancelotWindow::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);

    m_corona->
        setSceneRect(QRectF(QPointF(), event->size()));

    m_root->
        setGeometry(QRect(QPoint(), event->size()));

    resizeWindow();
}

#include "LancelotWindow.moc"

