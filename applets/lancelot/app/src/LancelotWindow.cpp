#include "LancelotWindow.h"
#include <kwindowsystem.h>

#include <KRecentDocument>
#include <plasma/animator.h>
#include <plasma/phase.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <kworkspace/kworkspace.h>
#include "ksmserver_interface.h"
#include "screensaver_interface.h"
#include "krunner_interface.h"

#define windowHeight 500
#define sectionsWidth 128
#define mainWidth 422

#define HIDE_TIMER_INTERVAL 1500
#define SEARCH_TIMER_INTERVAL 300

// QGV with a custom background painted

class CustomGraphicsView : public QGraphicsView {
public:
    CustomGraphicsView  ( QWidget * parent = 0 )
        : QGraphicsView(parent), m_background(NULL) {}
    CustomGraphicsView  ( QGraphicsScene * scene, QWidget * parent = 0 )
        : QGraphicsView(scene, parent), m_background(NULL) {}
    void drawBackground (QPainter * painter, const QRectF & rect)
    {
        Q_UNUSED(rect);
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(QRectF(rect.x()-2,rect.y()-2,rect.width()+2,rect.height()+2).toRect(), Qt::transparent);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        if (m_background) {
            m_background->resize(size());
            m_background->paint(painter, 0, 0, "background");
        }
    }
private:
    Plasma::Svg * m_background;
    friend class LancelotWindow;
};

// Window

LancelotWindow::LancelotWindow()
{
    setFocusPolicy(Qt::WheelFocus);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky);
    
    connect(& m_searchTimer, SIGNAL(timeout()), this, SLOT(doSearch()));
    m_searchTimer.setInterval(SEARCH_TIMER_INTERVAL);
    m_searchTimer.setSingleShot(true);

    connect(& m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    m_hideTimer.setInterval(HIDE_TIMER_INTERVAL);
    m_hideTimer.setSingleShot(true);

    m_corona = new Plasma::Corona(this);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins (0, 0, 0, 0);
    setLayout(m_layout);

    m_view = new CustomGraphicsView(m_corona, this);
    m_view->setWindowFlags(Qt::FramelessWindowHint);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //m_view->m_background = new Plasma::Svg("lancelot/theme");
    //m_view->m_background->setContentType(Plasma::Svg::ImageSet);

    m_layout->addWidget(m_view);
    
    instance = new Lancelot::Instance();

    m_root = new Lancelot::Panel("m_root"); //new RootWidget(); //Applet(0, "plasma_applet_clock");
    ((Lancelot::Panel * )m_root)->setBackground("lancelot/main-background");
    m_corona->addItem(m_root);
    
    /* Dirty hack to get an edit box before Qt 4.4 :: begin */
    _m_view = m_view;
    _m_root = m_root;
    /* Dirty hack to get an edit box before Qt 4.4 :: end */
    
    setupUi(m_root);
    ((Lancelot::Panel * )m_root)->setLayout(layoutMain);
    centerBackground->disable();
    
    /* Dirty hack to get an edit box before Qt 4.4 :: begin */
    editSearch->setParent(this);
    /* Dirty hack to get an edit box before Qt 4.4 :: end */

    instance->activateAll();
    
    m_sectionsSignalMapper = new QSignalMapper(this);
    connect (m_sectionsSignalMapper,
        SIGNAL(mapped(const QString &)),
        this,
        SLOT(sectionActivated(const QString &))
    );

    QMapIterator<QString, Lancelot::ToggleExtenderButton * > i(sectionButtons);
    while (i.hasNext()) {
        i.next();
        connect(i.value(), SIGNAL(activated()), m_sectionsSignalMapper, SLOT(map()));
        m_sectionsSignalMapper->setMapping(i.value(), i.key());
    }

    connect(buttonSystemLockScreen, SIGNAL(activated()), this, SLOT(systemLock()));
    connect(buttonSystemLogout,     SIGNAL(activated()), this, SLOT(systemLogout()));
    connect(buttonSystemSwitchUser, SIGNAL(activated()), this, SLOT(systemSwitchUser()));
    
    connect(editSearch,
        SIGNAL(textChanged(const QString &)),
        this, SLOT(search(const QString &))
    );

}

LancelotWindow::~LancelotWindow()
{
}

void LancelotWindow::lancelotShow(int x, int y)
{
    panelSections->show();
    layoutMain->setSize(sectionsWidth, Plasma::LeftPositioned);
    layoutMain->updateGeometry();
    
    resizeWindow(QSize(mainWidth + sectionsWidth, windowHeight));
    
    showWindow(x, y);
}

void LancelotWindow::lancelotShowItem(int x, int y, QString name)
{
    sectionActivated(name);

    panelSections->hide();
    layoutMain->setSize(0, Plasma::LeftPositioned);
    layoutMain->updateGeometry();
    
    resizeWindow(QSize(mainWidth, windowHeight));
    
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

void LancelotWindow::showWindow(int x, int y)
{
    m_hideTimer.stop();

    if (isVisible()) {
        return;
    }

    
    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(x, y));

    Plasma::Flip flip = Plasma::VerticalFlip;
    
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
    
    layoutMain->setFlip(flip);
    layoutMain->invalidate();

    show();
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    
    //KWindowSystem::activateWindow(winId());
    KWindowSystem::forceActiveWindow(winId());
    
    // TODO: make this // editSearch->setFocus();
    move(x, y);
}

void LancelotWindow::resizeWindow(QSize newSize)
{
    //kDebug()
    m_view->resetCachedContent();

    resize(newSize.width(), newSize.height());
    m_view->resize(newSize.width(), newSize.height());

    m_corona->setSceneRect(QRectF(0, 0, newSize.width(), newSize.height()));
    layoutMain->setGeometry(QRectF(0, 0, newSize.width(), newSize.height()));
    layoutMain->updateGeometry();

    m_view->invalidateScene();
    m_view->update();
    
    m_root->setGeometry(QRect(QPoint(), newSize));
    //layoutMain->setGeometry(QRect(QPoint(), newSize));
    //layoutMain->invalidate();

    update();
}

void LancelotWindow::focusOutEvent(QFocusEvent * event) {
    Q_UNUSED(event);
    hide();
}

void LancelotWindow::leaveEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = false;
    m_hideTimer.start();
}

void LancelotWindow::enterEvent(QEvent * event) {
    Q_UNUSED(event);
    m_hovered = true;
    m_hideTimer.stop();
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

void LancelotWindow::sectionActivated(const QString & item) {
    foreach (Lancelot::ToggleExtenderButton * button, sectionButtons) {
        button->setPressed(false);
    }
    if (sectionButtons.contains(item)) {
        sectionButtons[item]->setPressed(true);
    }
    layoutCenter->show(item);
}

void LancelotWindow::search(const QString & string)
{
    if (editSearch->text() != string) {
        editSearch->setText(string);
    }
    m_searchTimer.stop();
    m_searchString = string;
    m_searchTimer.start();
}

void LancelotWindow::doSearch()
{
    m_searchTimer.stop();
    //m_runnerModel->setSearchString(m_searchString);
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
    hide();
    QTimer::singleShot(500, this, SLOT(systemDoSwitchUser()));
}

void LancelotWindow::systemDoSwitchUser()
{
    org::kde::krunner::Interface krunner("org.kde.krunner", "/Interface", QDBusConnection::sessionBus());
    
    if (krunner.isValid()) {
        krunner.switchUser();
    }
}

#include "LancelotWindow.moc"
