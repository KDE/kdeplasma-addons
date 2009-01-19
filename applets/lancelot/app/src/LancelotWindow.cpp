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
#include "ksmserver_interface.h"
#include "screensaver_interface.h"
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
#include "models/ContactsKopete.h"
#include "models/MessagesKmail.h"

#include "models/BaseMergedModel.h"

#include <lancelot/widgets/ResizeBordersPanel.h>
#include <lancelot/widgets/PopupWidget.h>

#include <KLineEdit>
#include <Plasma/LineEdit>
#include <KAboutApplicationDialog>
#include <KCmdLineArgs>

#define sectionsWidth 128
#define windowHeightDefault 500
#define mainWidthDefault    422

#define HIDE_TIMER_INTERVAL 1500
#define SEARCH_TIMER_INTERVAL 300

#define Merged(A) ((Models::BaseMergedModel *)(A))

// Transparent QGV

class CustomGraphicsView : public QGraphicsView {
public:
    CustomGraphicsView(
        QGraphicsScene * scene,
        LancelotWindow * parent
    ) : QGraphicsView(scene, parent),
        m_parent(parent), m_resizing(false),
        m_cache(NULL)
    {
        setWindowFlags(Qt::FramelessWindowHint);
        setFrameStyle(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_fgBrush = QBrush(QColor(0, 0, 0, 200));
    }

    void drawItems (QPainter * painter, int numItems,
            QGraphicsItem ** items, const QStyleOptionGraphicsItem * options )
    {
        if (m_resizing) {
            painter->drawPixmap(0, 0, m_cache->scaled(size()));
        } else {
            QGraphicsView::drawItems(painter, numItems, items, options);
        }
    }

    void drawBackground(QPainter * painter, const QRectF & rect)
    {
        Q_UNUSED(rect);
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
    }

    void drawForeground(QPainter * painter, const QRectF & rect)
    {
        if (m_resizing) {
            painter->fillRect(rect, m_fgBrush);
        }
    }

//    void drawItems(QPainter * painter, int numItems,
//            QGraphicsItem ** items,  const QStyleOptionGraphicsItem * options)
//    {
//        if (!m_resizing) {
//            QGraphicsView::drawItems(painter, numItems, items, options);
//        }
//    }

#define passEvent(Event)                           \
    void Event(QMouseEvent *e) {                   \
        m_parent->Event(e);                        \
        QGraphicsView::Event(e);                   \
    }

    passEvent(mousePressEvent)
    passEvent(mouseReleaseEvent)
    passEvent(mouseMoveEvent)

#undef passEvent

    void resize(QSize newSize = QSize()) {

        if (newSize == QSize()) {
            newSize = size();
        }

        QGraphicsView::resize(newSize.width(), newSize.height());

        if (!m_resizing) {
            resetCachedContent();
            m_parent->m_corona->
                setSceneRect(QRectF(0, 0, newSize.width(), newSize.height()));

            m_parent->m_root->
                setGeometry(QRect(QPoint(), newSize));

            invalidateScene();
        }
        update();
    }

    void startResizing()
    {
        m_cache = new QPixmap(size());
        QPainter p(m_cache);
        render(&p);
        m_resizing = true;
    }

    void stopResizing()
    {
        m_resizing = false;
        delete m_cache;
        resize();
    }

private:
    LancelotWindow  * m_parent;
    QGraphicsWidget * m_root;
    QBrush            m_fgBrush;

    bool             m_resizing;
    QPixmap        * m_cache;

    friend class LancelotWindow;
};

// Window

LancelotWindow::LancelotWindow()
    : m_root(NULL), m_view(NULL), m_corona(NULL), m_layout(NULL),
    m_hovered(false), m_showingFull(true), m_sectionsSignalMapper(NULL),
    m_config("lancelotrc"), m_mainConfig(&m_config, "Main"),
    instance(NULL),
    m_configWidget(NULL),
    m_resizeDirection(None),
    m_mainSize(mainWidthDefault, windowHeightDefault),
    m_skipEvent(false)
{
    setFocusPolicy(Qt::WheelFocus);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky);

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hideImmediate()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    m_hideTimer.setSingleShot(true);

    m_corona = new Plasma::Corona(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins (0, 0, 0, 0);
    setLayout(m_layout);

    m_view = new CustomGraphicsView(m_corona, this);
    m_layout->addWidget(m_view);

    instance = new Lancelot::Instance();
    instance->setHasApplication(true);

    m_root = new Lancelot::ResizeBordersPanel();

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
    m_view->installEventFilter(this);

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

    connect(buttonSystemLockScreen, SIGNAL(activated()), this, SLOT(systemLock()));
    connect(buttonSystemLogout,     SIGNAL(activated()), this, SLOT(systemLogout()));
    connect(buttonSystemSwitchUser, SIGNAL(activated()), this, SLOT(systemSwitchUser()));

    connect(buttonLancelotContext,  SIGNAL(activated()), this, SLOT(lancelotContext()));

    connect(editSearch->widget(),
        SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

    instance->activateAll();
    loadConfig();
    setupActions();

    /* testing */
    /*
    Lancelot::PopupWidget * popup = new Lancelot::PopupWidget();
    QGraphicsScene * scene = new QGraphicsScene();

    Lancelot::ExtenderButton * button = new Lancelot::ExtenderButton(
            "Title", "Description");
    scene->addItem(button);
    button->setGroupByName("SystemButtons");
    button->setExtenderPosition(Lancelot::NoExtender);
    popup->setGraphicsWidget(button);
    // popup->resize(200, 400);
    popup->show();
    */
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
            x = screenRect.right() - width();
            flip |= Plasma::HorizontalFlip;
        }

        if (y + height() > screenRect.bottom()) {
            y = screenRect.bottom() - height();
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
    resizeWindow();

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
    //m_view->setFocus();
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
    m_view->resize(newSize);
    m_root->group()->backgroundSvg()->resizeFrame(newSize);
    setMask(m_root->group()->backgroundSvg()->mask());
}

void LancelotWindow::leaveEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = false;
    if (m_resizeDirection != None) {
        m_hideTimer.start();
    }
    QWidget::leaveEvent(event);
}

void LancelotWindow::enterEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = true;
    m_hideTimer.stop();
    QWidget::enterEvent(event);
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
    res << "Applications" << "Computer" << "Contacts" << "Documents";
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
    kDebug() << item;
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
    sectionActivated("search");
}

void LancelotWindow::systemLock()
{
    lancelotHide(true);
    QTimer::singleShot(500, this, SLOT(systemDoLock()));
}

void LancelotWindow::systemLogout()
{
    lancelotHide(true);
    QTimer::singleShot(500, this, SLOT(systemDoLogout()));
}

void LancelotWindow::systemDoLock()
{
    org::freedesktop::ScreenSaver screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", QDBusConnection::sessionBus());

    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

void LancelotWindow::systemDoLogout()
{
    org::kde::KSMServerInterface smserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    if (smserver.isValid()) {
        smserver.logout(
            KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault,
            KWorkSpace::ShutdownModeDefault
        );
    }
}

void LancelotWindow::systemSwitchUser()
{
    search("SESSIONS");
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

     m_models["Contacts"]          = new Models::ContactsKopete();
     m_models["Messages"]          = new Models::MessagesKmail();

     m_models["Runner"]            = new Models::Runner();

     // Groups:

     m_modelGroups["ComputerLeft"]   = new Models::BaseMergedModel();
     m_modelGroups["DocumentsLeft"]  = new Models::BaseMergedModel();
     m_modelGroups["ContactsLeft"]   = new Models::BaseMergedModel();

     m_modelGroups["ComputerRight"]  = new Models::BaseMergedModel();
     m_modelGroups["DocumentsRight"] = new Models::BaseMergedModel();
     m_modelGroups["ContactsRight"]  = new Models::BaseMergedModel();

     // Assignments: Model - Group:
     // defined Merged(A) ((Lancelot::MergedActionListViewModel *)(A))

     Merged(m_modelGroups["ComputerLeft"])->addModel ("Places", QIcon(), i18n("Places"), m_models["Places"]);
     Merged(m_modelGroups["ComputerLeft"])->addModel ("System", QIcon(), i18n("System"), m_models["SystemServices"]);

     Merged(m_modelGroups["ComputerRight"])->addModel ("Devices/Removable", QIcon(), i18n("Removable"), m_models["Devices/Removable"]);
     Merged(m_modelGroups["ComputerRight"])->addModel ("Devices/Fixed", QIcon(), i18n("Fixed"), m_models["Devices/Fixed"]);

     Merged(m_modelGroups["DocumentsLeft"])->addModel ("NewDocuments", QIcon(), i18n("New:"), m_models["NewDocuments"]);

     Merged(m_modelGroups["DocumentsRight"])->addModel("OpenDocuments", QIcon(), i18nc("@title Title of a list of documents that are open", "Open documents"), m_models["OpenDocuments"]);
     Merged(m_modelGroups["DocumentsRight"])->addModel("RecentDocuments", QIcon(), i18n("Recent documents"), m_models["RecentDocuments"]);

     Merged(m_modelGroups["ContactsLeft"])->addModel("Messages", QIcon(), i18n("Unread messages"), m_models["Messages"]);
     Merged(m_modelGroups["ContactsRight"])->addModel("Contacts", QIcon(), i18n("Contacts"), m_models["Contacts"]);

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
        m_view->startResizing();
    }

    QWidget::mousePressEvent(e);
}

void LancelotWindow::mouseReleaseEvent(QMouseEvent * e)
{
    if (m_resizeDirection != None) {
        m_mainConfig.writeEntry("width",  m_mainSize.width());
        m_mainConfig.writeEntry("height", m_mainSize.height());
        m_mainConfig.sync();
        m_resizeDirection = None;

        //resizeWindow();
        m_view->stopResizing();
    }
    QWidget::mouseReleaseEvent(e);
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

        setGeometry(QRect(newWindowPosition, newSize));
    }
    QWidget::mouseMoveEvent(e);
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
    m_mainSize = QSize(
        m_mainConfig.readEntry("width",  mainWidthDefault),
        m_mainConfig.readEntry("height", windowHeightDefault)
    );

    if (m_configWidget == NULL) {
        m_configWidget = new QWidget();
        m_configUi.setupUi(m_configWidget);
    }
    m_configUi.loadConfig();

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
    } else {
        instance->group("ActionListView-Left")
            ->setProperty("ExtenderPosition", Lancelot::NoExtender);
        instance->group("ActionListView-Right")
            ->setProperty("ExtenderPosition", Lancelot::NoExtender);
        instance->group("PassagewayView")
            ->setProperty("ActivationMethod", Lancelot::ClickActivate);
    }
    instance->group("ActionListView-Left")->notifyUpdated();
    instance->group("ActionListView-Right")->notifyUpdated();
    instance->group("PassagewayView")->notifyUpdated();

    if (m_configUi.appbrowserColumnLimitted()) {
        passagewayApplications->setColumnLimit(2);
    } else {
        passagewayApplications->setColumnLimit(22); // TODO: Temp
    }
}

void LancelotWindow::lancelotContext()
{
    QMenu menu;
    connect(
            menu.addAction(KIcon(),
                i18n("Menu Editor")), SIGNAL(triggered(bool)),
            this, SLOT(showMenuEditor()));

    connect(
            menu.addAction(KIcon("configure-shortcuts"),
                i18n("Configure &Shortcuts...")), SIGNAL(triggered(bool)),
            this, SLOT(configureShortcuts()));

    connect(
            menu.addAction(KIcon("configure"),
                i18n("Configure &Lancelot menu...")), SIGNAL(triggered(bool)),
            this, SLOT(configureMenu()));

    connect(
            menu.addAction(KIcon("lancelot"),
                i18n("&About Lancelot")), SIGNAL(triggered(bool)),
            this, SLOT(showAboutDialog()));

    menu.exec(QCursor::pos());
}

void LancelotWindow::configureMenu()
{
    lancelotHide(true);
    const QString dialogID = "LancelotMenyConfigurationDialog";
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

#include "LancelotWindow.moc"

