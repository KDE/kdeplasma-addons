/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotWindow.h"
#include <kwindowsystem.h>

#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>

#include <KRecentDocument>
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

#include <KLineEdit>
#include <Plasma/LineEdit>

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

            //layoutMain->setGeometry(QRectF(0, 0, newSize.width(), newSize.height()));
            //layoutMain->updateGeometry();

            m_parent->m_root->
                setGeometry(QRect(QPoint(), newSize));

            invalidateScene();
        }
        update();
    }

    void startResizing()
    {
        m_cache = new QPixmap(size());
        render(& QPainter(m_cache));
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
    instance(NULL), m_resizeDirection(None),
    m_mainSize(mainWidthDefault, windowHeightDefault)
{
    setFocusPolicy(Qt::WheelFocus);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky);

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    m_hideTimer.setSingleShot(true);

    m_corona = new Plasma::Corona(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins (0, 0, 0, 0);
    setLayout(m_layout);

    m_view = new CustomGraphicsView(m_corona, this);
    m_layout->addWidget(m_view);

    instance = new Lancelot::Instance();

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

    passagewayApplications->setEntranceTitle(i18n("Favorites"));
    passagewayApplications->setEntranceIcon(KIcon("favorites"));
    passagewayApplications->setAtlasTitle(i18n("Applications"));
    passagewayApplications->setAtlasIcon(KIcon("applications-other"));
    /* End TODO */

    instance->activateAll();

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

    connect(editSearch->widget(),
        SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

    loadConfig();
}

LancelotWindow::~LancelotWindow()
{
    delete instance;
}

void LancelotWindow::loadConfig()
{
    m_mainSize = QSize(
        m_mainConfig.readEntry("width",  mainWidthDefault),
        m_mainConfig.readEntry("height", windowHeightDefault)
    );
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

    instance->group("SystemButtons")->setProperty("ExtenderPosition", QVariant(
            (flip & Plasma::VerticalFlip)?(Lancelot::TopExtender):(Lancelot::BottomExtender)
    ));
    instance->group("SystemButtons")->notifyUpdated();

    if (m_showingFull) {
        sectionActivated("applications");
    }

    move(x, y);
    show();
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    KWindowSystem::forceActiveWindow(winId());
    editSearch->setFocus();
    editSearch->setText(QString());

}

void LancelotWindow::resizeWindow()
{
    QSize newSize = m_mainSize;
    if (m_showingFull) {
        newSize.rwidth() += sectionsWidth;
    }
    resize(newSize);
    m_view->resize(newSize);
}

void LancelotWindow::focusOutEvent(QFocusEvent * event) {
    Q_UNUSED(event);
    hide();
    QWidget::focusOutEvent(event);
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
    foreach (Lancelot::ExtenderButton * button, sectionButtons) {
        button->setChecked(false);
    }

    if (sectionButtons.contains(item)) {
        sectionButtons[item]->setChecked(true);
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
    hide();
    QTimer::singleShot(500, this, SLOT(systemDoLock()));
}

void LancelotWindow::systemLogout()
{
    hide();
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
    kDebug() << "Do Logout!!!";

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
    //  // For now, we do not use systems switch function, but rather
    //  // SESSIONS runner...
    //  // hide();
    //  // QTimer::singleShot(500, this, SLOT(systemDoSwitchUser()));
    search("SESSIONS");
}

//  void LancelotWindow::systemDoSwitchUser()
//  {
//      org::kde::krunner::Interface krunner("org.kde.krunner", "/Interface", QDBusConnection::sessionBus());
//
//      if (krunner.isValid()) {
//          krunner.switchUser();
//      }
//  }

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

     Merged(m_modelGroups["DocumentsRight"])->addModel("OpenDocuments", QIcon(), i18n("Open documents"), m_models["OpenDocuments"]);
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
         new Lancelot::PassagewayViewModelProxy(
             Models::FavoriteApplications::instance(), i18n("Favorites"), KIcon("favorites")
         )
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

        move(newWindowPosition);
        resizeWindow();
    }
    QWidget::mouseMoveEvent(e);
}

bool LancelotWindow::eventFilter(QObject * object, QEvent * event)
{
    kDebug() << event;
    if (object == editSearch->nativeWidget() && event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            lancelotHide(true);
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}

#include "LancelotWindow.moc"

