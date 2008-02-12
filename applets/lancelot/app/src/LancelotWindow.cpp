#include "LancelotWindow.h"
#include <kwindowsystem.h>

#define windowHeight 500
#define sectionsWidth 128
#define mainWidth 422

#define HIDE_TIMER_INTERVAL 1500

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

class RootWidget: public Plasma::Widget {
    protected:
};

// Window

LancelotWindow::LancelotWindow()
{
    setFocusPolicy(Qt::WheelFocus);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);// | Qt::Popup);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky);
    
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

    m_view->m_background = new Plasma::Svg("lancelot/theme");
    m_view->m_background->setContentType(Plasma::Svg::ImageSet);

    m_layout->addWidget(m_view);
    
    instance = new Lancelot::Instance();

    m_root = new RootWidget(); //Applet(0, "plasma_applet_clock");
    m_corona->addItem(m_root);

    setupUi(m_root);
    
    //m_root->setLayout(layoutMain);
}

LancelotWindow::~LancelotWindow()
{
}

void LancelotWindow::lancelotShow(int x, int y)
{
    // TODO: make this // panelSections->show();
    layoutMain->setSize(sectionsWidth, Plasma::LeftPositioned);
    layoutMain->updateGeometry();
    
    resizeWindow(QSize(mainWidth + sectionsWidth, windowHeight));
    
    if (!isVisible()) {
        showWindow(x, y);
    }
}

void LancelotWindow::lancelotShowItem(int x, int y, QString name)
{
    // TODO: make this // panelSections->show();
    layoutMain->setSize(0, Plasma::LeftPositioned);
    layoutMain->updateGeometry();
    
    resizeWindow(QSize(mainWidth, windowHeight));
    
    if (!isVisible()) {
        showWindow(x, y);
    }
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

void LancelotWindow::search(const QString & string)
{
    
}

void LancelotWindow::showWindow(int x, int y)
{
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
    m_hideTimer.stop();
    
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
    layoutMain->setGeometry(QRect(QPoint(), newSize));
    layoutMain->invalidate();

    update();
}

void LancelotWindow::focusOutEvent(QFocusEvent * event) {
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

