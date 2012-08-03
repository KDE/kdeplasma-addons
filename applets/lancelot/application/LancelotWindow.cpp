/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <QDesktopWidget>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <KAuthorized>
#include <KCmdLineArgs>
#include <KConfigDialog>
#include <KDebug>
#include <KGlobalAccel>
#include <KLineEdit>
#include <KProcess>
#include <KRecentDocument>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KWindowSystem>

#include <kworkspace/kworkspace.h>

#include <Plasma/LineEdit>
#include <Plasma/WindowEffects>

#include <Lancelot/PlasmaServiceListModel>
#include <Lancelot/PopupMenu>
#include <Lancelot/TabBar>
#include <Lancelot/ResizeBordersPanel>

#include <Lancelot/Models/Serializator>
#include <Lancelot/Models/Devices>
#include <Lancelot/Models/Places>
#include <Lancelot/Models/SystemServices>
#include <Lancelot/Models/RecentDocuments>
#include <Lancelot/Models/OpenDocuments>
#include <Lancelot/Models/NewDocuments>
#include <Lancelot/Models/FolderModel>
#include <Lancelot/Models/FavoriteApplications>
#include <Lancelot/Models/Applications>
#include <Lancelot/Models/Runner>
#include <Lancelot/Models/SystemActions>
#include <Lancelot/Models/ContactsKopete>
#include <Lancelot/Models/MessagesKmail>
#include <Lancelot/Models/BaseMergedModel>
#include <Lancelot/Models/Logger>

#define sectionsWidth \
    (m_mainConfig.readEntry("collapseSections", false) ? 64 : 128)

#define windowHeightDefault 500
#define mainWidthDefault    422

#define HIDE_TIMER_INTERVAL 1500
#define SEARCH_TIMER_INTERVAL 300

LancelotWindow::LancelotWindow()
    : m_root(NULL), m_corona(NULL),
    m_hovered(false), m_showingFull(true), m_sectionsSignalMapper(NULL),
    m_config("lancelotrc"), m_mainConfig(&m_config, "Main"),
    m_configWidget(NULL),
    m_resizeDirection(None),
    m_mainSize(mainWidthDefault, windowHeightDefault),
    m_skipEvent(false),
    menuSystemButton(NULL),
    menuLancelotContext(NULL),
    m_cachedOpenPosition(-1, -1),
    m_cachedWindowSize(-1, -1),
    m_cachedFlip(0),
    m_cachedOpenPositionCentered(false),
    m_cachedShowingFull(false),
    m_firstOpen(true)
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
    editSearch->nativeWidget()->setClickMessage(i18nc("Enter the text to search for", "Search..."));
    editSearch->nativeWidget()->setContextMenuPolicy(Qt::NoContextMenu);
    editSearch->show();
    layoutSearch->addItem(editSearch,
        Lancelot::NodeLayout::NodeCoordinate(0.0, 0.5, 0, 0),
        Lancelot::NodeLayout::NodeCoordinate(1.0, 0.5, 0, QREAL_INFINITY)
    );
    editSearch->nativeWidget()->installEventFilter(this);
    editSearch->setFocusPolicy(Qt::WheelFocus);
    editSearch->nativeWidget()->setFocusPolicy(Qt::WheelFocus);

    m_completion = new KCompletion();
    editSearch->nativeWidget()->setCompletionObject(m_completion);
    editSearch->nativeWidget()->setCompletionMode(
        KGlobalSettings::CompletionMan);
    m_completion->insertItems(m_configUi.searchHistory());

    passagewayApplications->setEntranceTitle(i18n("Favorites"));
    passagewayApplications->setEntranceIcon(KIcon("favorites"));
    passagewayApplications->setAtlasTitle(i18n("Applications"));
    passagewayApplications->setAtlasIcon(KIcon("applications-other"));

    tabbarSections->installEventFilter(this);
    tabbarSections->setTabsGroupName("SectionButtons");
    tabbarSections->addTab("documents",    KIcon("applications-office"), i18n("&Documents"),    "text/x-lancelotpart",
            "model=NewDocuments&type=list&version=1.0\nmodel=OpenDocuments&type=list&version=1.0\nmodel=RecentDocuments&type=list&version=1.0");
    tabbarSections->addTab("contacts",     KIcon("kontact"),             i18n("&Contacts"),
            "text/x-lancelotpart", "model=Messages&type=list&version=1.0\nmodel=Contacts&type=list&version=1.0");
    tabbarSections->addTab("computer",     KIcon("computer-laptop"),     i18n("Co&mputer"),     "text/x-lancelotpart",
            "model=Places&type=list&version=1.0\nmodel=System&type=list&version=1.0\nmodel=Devices%2FRemovable&type=list&version=1.0\nmodel=Devices%2FFixed&type=list&version=1.0");
    tabbarSections->addTab("applications", KIcon("applications-other"),  i18n("&Applications"), "text/x-lancelotpart",
            "model=FavoriteApplications&type=list&version=1.0\nmodel=Folder%20applications%3A%2F&type=list&version=1.0");
    tabbarSections->setOrientation(Qt::Vertical);
    tabbarSections->setTextDirection(Qt::Vertical);
    /* End TODO */

    connect(tabbarSections, SIGNAL(currentTabChanged(QString)),
            layoutCenter, SLOT(showCard(QString)));

    m_sectionsSignalMapper = new QSignalMapper(this);
    connect (m_sectionsSignalMapper,
        SIGNAL(mapped(QString)),
        this,
        SLOT(sectionActivated(QString))
    );

    connect(buttonSystem1, SIGNAL(activated()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem2, SIGNAL(activated()), this, SLOT(systemButtonClicked()));
    connect(buttonSystem3, SIGNAL(activated()), this, SLOT(systemButtonClicked()));

    connect(buttonLancelotContext,  SIGNAL(activated()), this, SLOT(lancelotContext()));

    connect(editSearch->widget(),
        SIGNAL(textChanged(QString)),
        this, SLOT(search(QString))
    );

    loadConfig();
    setupActions();

    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(focusChanged(QWidget*,QWidget*)));

    setStyleSheet("LancelotWindow { background: black }");
}

void LancelotWindow::drawBackground(QPainter * painter, const QRectF & rect)
{
    painter->setCompositionMode(QPainter::CompositionMode_Clear);
    painter->fillRect(
        QRectF(rect.x() - 2, rect.y() - 2,
               rect.width() + 2, rect.height() + 2).toRect(),
               Qt::transparent);
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

void LancelotWindow::toggleCollapsedSections()
{
    updateCollapsedSections(true);
}

void LancelotWindow::updateCollapsedSections(bool toggle)
{
    bool collapseSections = m_mainConfig.readEntry("collapseSections", false);
    if (toggle) {
        collapseSections = !collapseSections;
        m_mainConfig.writeEntry("collapseSections", collapseSections);
        m_mainConfig.sync();
    }

    layoutMain->setSize((m_showingFull ? sectionsWidth : 0), Lancelot::FullBorderLayout::LeftBorder);

    if (collapseSections) {
        tabbarSections->setTabIconSize(QSize(32, 32));
        tabbarSections->setTextDirection(Qt::Horizontal);
    } else {
        tabbarSections->setTabIconSize(QSize(48, 48));
        tabbarSections->setTextDirection(Qt::Vertical);
    }

    updateWindowSize();
}

LancelotWindow::~LancelotWindow()
{
    m_configUi.setSearchHistory(m_completion->items());
    delete m_configWidget;
}

void LancelotWindow::lancelotShow(int x, int y)
{
    m_showingFull = true;
    showWindow(x, y);
}

void LancelotWindow::lancelotToggleCentered()
{
    if (isVisible()) {
        hide();
    } else lancelotShowCentered();
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
        QTimer::singleShot(100, editSearch->nativeWidget(), SLOT(clear()));
        // editSearch->setText(QString());

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
    kDebug() << "=== showing the window";

    if (m_firstOpen || m_cachedShowingFull == m_showingFull) {
        m_cachedShowingFull = m_showingFull;

        kDebug() << "cache for m_showingFull not used";
        tabbarSections->setVisible(m_showingFull);
        layoutMain->setSize((m_showingFull ? sectionsWidth : 0), Lancelot::FullBorderLayout::LeftBorder);

        // in this case, we should really update everything...
        m_firstOpen = true;
    }

    updateCollapsedSections();

    m_resizeDirection = None;
    m_hideTimer.stop();

    if (isVisible()) {
        // We are exiting because we do not want to move already opened window
        // because most probably it is just invoked from the same applet and
        // needs to show only another category
        updateWindowSize();
        return;
    }

    if (centered) {
        x = QCursor::pos().x();
        y = QCursor::pos().y();
    }

    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(x, y));

    updateWindowSize();

    if (m_firstOpen ||
            (m_cachedOpenPosition != QPoint(x, y) || m_cachedOpenPositionCentered != centered)) {
        kDebug() << "cache for position not used";

        m_cachedOpenPosition = QPoint(x, y);
        m_cachedOpenPositionCentered = centered;

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

            if (m_showingFull) {
                if (flip & Plasma::HorizontalFlip) {
                    x += sectionsWidth / 2;
                } else {
                    x -= sectionsWidth / 2;
                }
            }
        } else {
            flip = Plasma::NoFlip;

            x = screenRect.left()
                + (screenRect.width() - width()) / 2;
            y = screenRect.top()
                + (screenRect.height() - height()) / 2;
        }

        move(x, y);

        if (m_firstOpen || m_cachedFlip != flip) {
            kDebug() << "cache for flip not used";
            m_cachedFlip = flip;

            kDebug() << "Flip:" << flip;
            layoutMain->setFlip(flip);
            tabbarSections->setFlip(flip);

            if (m_configUi.activationMethod() == LancelotConfig::NoClick) {
                Lancelot::Global::self()->group("SystemButtons")->setProperty("extenderPosition", QVariant(
                        (flip & Plasma::VerticalFlip)?(Lancelot::TopExtender):(Lancelot::BottomExtender)
                ));
                Lancelot::Global::self()->group("SystemButtons")
                    ->setProperty("activationMethod", Lancelot::ExtenderActivate);
            } else {
                Lancelot::Global::self()->group("SystemButtons")
                    ->setProperty("extenderPosition", QVariant(Lancelot::NoExtender));
                Lancelot::Global::self()->group("SystemButtons")
                    ->setProperty("activationMethod", Lancelot::ClickActivate);
            }
            Lancelot::Global::self()->group("LancelotContext")->setProperty("extenderPosition",
                    Lancelot::Global::self()->group("SystemButtons")->property("extenderPosition"));
            Lancelot::Global::self()->group("LancelotContext")->setProperty("activationMethod",
                    Lancelot::Global::self()->group("SystemButtons")->property("activationMethod"));
            Lancelot::Global::self()->group("SystemButtons")->notifyUpdated();
            Lancelot::Global::self()->group("LancelotContext")->notifyUpdated();
        }
    }

    if (m_showingFull) {
        sectionActivated("applications");
    }

    if (KWindowSystem::compositingActive()) {
        if (m_cachedFlip & Plasma::VerticalFlip) {
            Plasma::WindowEffects::slideWindow(this, Plasma::TopEdge);

        } else {
            Plasma::WindowEffects::slideWindow(this, Plasma::BottomEdge);
        }
    }

    show();

    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove );
    KWindowSystem::forceActiveWindow(winId());

    //editSearch->clearFocus();
    editSearch->nativeWidget()->setFocus();
    editSearch->setFocus();

    m_firstOpen = false;
}

bool LancelotWindow::updateWindowSize()
{
    QSize newSize = m_mainSize;
    if (m_showingFull) {
        newSize.rwidth() += sectionsWidth;
    }

    if (!m_firstOpen && newSize == m_cachedWindowSize) {
        return false;
    }

    kDebug() << "cache for size not used" << newSize << m_cachedWindowSize;
    m_cachedWindowSize = newSize;

    resize(newSize);
    m_corona->
        setSceneRect(QRectF(QPointF(), newSize));

    m_root->
        setGeometry(QRect(QPoint(), newSize));

    resize(newSize);
    m_root->group()->backgroundSvg()->resizeFrame(newSize);

    const QRegion & mask = m_root->group()->backgroundSvg()->mask();
    // setMask(mask);
    Plasma::WindowEffects::enableBlurBehind(winId(), true, mask);
    Plasma::WindowEffects::overrideShadow(winId(), true);

    return true;
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
    res << "kde" << "computer-laptop" << "kontact" << "applications-office";
    // res << "applications-other" << "computer-laptop" << "kontact" << "applications-office";
    return res;
}

QString LancelotWindow::currentSection()
{
    if (isHidden()) {
        return QString();
    }

    return tabbarSections->currentTab();
}

void LancelotWindow::sectionActivated(const QString & item)
{
    kDebug() << item;
    tabbarSections->setCurrentTab(item);

    // TODO: m_activeSection should be examined
    // - since we have a tabbarSections, it is not really
    // used and it is not representing the active section
    if (item == m_activeSection) {
        return;
    }

    m_activeSection = item;

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

    layoutCenter->showCard(item);
}

void LancelotWindow::search(const QString & string)
{
    if (editSearch->text() != string) {
        editSearch->setText(string);
    }

    m_searchString = string;

    ((Lancelot::Models::Runner *) m_models["Runner"])->setSearchString(m_searchString);
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
        Lancelot::Models::SystemActions::self()->action(systemButtonActions[button]);

    if (!model) return;

    if (!menuSystemButton) {
        menuSystemButton = new Lancelot::PopupList();
        menuSystemButton->resize(200, 200);
        Lancelot::Models::ApplicationConnector * ac = Lancelot::Models::ApplicationConnector::self();
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

    menuSystemButton->exec(menuSystemButton->pos());
}

void LancelotWindow::setupModels()
{
    // Models:
    m_models["Places"]            = new Lancelot::Models::Places();
    m_models["SystemServices"]    = new Lancelot::Models::SystemServices();
    m_models["Devices/Removable"] = new Lancelot::Models::Devices(Lancelot::Models::Devices::Removable);
    m_models["Devices/Fixed"]     = new Lancelot::Models::Devices(Lancelot::Models::Devices::Fixed);

    m_models["NewDocuments"]      = new Lancelot::Models::NewDocuments();
    m_models["RecentDocuments"]   = new Lancelot::Models::RecentDocuments();
    m_models["OpenDocuments"]     = new Lancelot::Models::OpenDocuments();

    m_models["Runner"]            = new Lancelot::Models::Runner(); //allowedRunners);

    // Groups:

    m_modelGroups["ComputerLeft"]   = new Lancelot::Models::BaseMergedModel();
    m_modelGroups["DocumentsLeft"]  = new Lancelot::Models::BaseMergedModel();
    m_modelGroups["ContactsLeft"]   = new Lancelot::Models::BaseMergedModel();

    m_modelGroups["ComputerRight"]  = new Lancelot::Models::BaseMergedModel();
    m_modelGroups["DocumentsRight"] = new Lancelot::Models::BaseMergedModel();
    m_modelGroups["ContactsRight"]  = new Lancelot::Models::BaseMergedModel();

    // Assignments: Model - Group:
    #define MergedAddModel(MergedModel, ModelID, Model, Title) \
        ((Lancelot::Models::BaseMergedModel *)(MergedModel))      \
        ->addModel((ModelID), QIcon(), Title, Model);

    MergedAddModel(m_modelGroups["ComputerLeft"], "Places", m_models["Places"], i18n("Places"));
    MergedAddModel(m_modelGroups["ComputerLeft"], "System", m_models["SystemServices"], i18n("System"));



    MergedAddModel(m_modelGroups["ComputerRight"], "Devices/Removable", m_models["Devices/Removable"], i18nc("@title Removable devices", "Removable"));
    MergedAddModel(m_modelGroups["ComputerRight"], "Devices/Fixed", m_models["Devices/Fixed"], i18nc("@title Fixed devices", "Fixed"));

    MergedAddModel(m_modelGroups["DocumentsLeft"], "NewDocuments", m_models["NewDocuments"], i18nc("@title New documents", "New"));

    MergedAddModel(m_modelGroups["DocumentsRight"], "OpenDocuments", m_models["OpenDocuments"], i18nc("@title List of open documents", "Open"));
    MergedAddModel(m_modelGroups["DocumentsRight"], "RecentDocuments", m_models["RecentDocuments"], i18nc("@title Recent documents", "Recent"));

    QString plugins;

    // Contacts Mail
    plugins = m_mainConfig.readEntry("mailPlugins", QString());
    if (plugins.isEmpty()) {
        m_models["Messages"]          = new Lancelot::Models::MessagesKmail();
        MergedAddModel(m_modelGroups["ContactsLeft"], "Messages", m_models["Messages"], i18n("Unread messages"));
    } else if (plugins != "disabled") {
        Lancelot::ActionListModel * model;
        foreach (const QString &plugin, plugins.split(',')) {
            model = new Lancelot::PlasmaServiceListModel(plugin);
            MergedAddModel(m_modelGroups["ContactsLeft"], model->selfTitle(), model, i18n("Unread messages"));
        }
    }

    // Contacts IM
    plugins = m_mainConfig.readEntry("imPlugins", QString());
    if (plugins.isEmpty()) {
        m_models["Contacts"]          = new Lancelot::Models::ContactsKopete();
        MergedAddModel(m_modelGroups["ContactsRight"], "Contacts", m_models["Contacts"], i18n("Online contacts"));
    } else if (plugins != "disabled") {
        Lancelot::ActionListModel * model;
        foreach (const QString &plugin, plugins.split(',')) {
            model = new Lancelot::PlasmaServiceListModel(plugin);
            MergedAddModel(m_modelGroups["ContactsRight"], model->selfTitle(), model, i18n("Online contacts"));
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
        new Lancelot::Models::FavoriteApplications::PassagewayViewProxy()
    );
    passagewayApplications->setAtlasModel(new Lancelot::Models::Applications());

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

        const QRegion & mask = m_root->group()->backgroundSvg()->mask();
        // setMask(mask);
        Plasma::WindowEffects::enableBlurBehind(winId(), true, mask);
        Plasma::WindowEffects::overrideShadow(winId(), true);
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
    // Right-click the tabbarSections
    if (event->type() == QEvent::GraphicsSceneMousePress &&
        object == tabbarSections) {
        QGraphicsSceneMouseEvent * mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            Lancelot::PopupMenu * menu = new Lancelot::PopupMenu(this);

            QString text;
            if (m_mainConfig.readEntry("collapseSections", false)) {
                text = i18n("Make buttons wider");
            } else {
                text = i18n("Make buttons narrower");
            }
            connect(
                    menu->addAction(KIcon(), text), SIGNAL(triggered(bool)),
                    this, SLOT(toggleCollapsedSections())
                    );

            menu->exec(QCursor::pos());
        }
    }

    // Catching key presses because no item has explicit
    // focus
    if (event->type() == QEvent::KeyPress) {
        bool pass = false;
        int oindex = m_focusIndex;
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Escape:
                lancelotHide(true);
                break;

            case Qt::Key_Tab:
            {
                QKeyEvent * endKeyEvent =
                    new QKeyEvent(QEvent::KeyPress, Qt::Key_End,
                               Qt::NoModifier);
                QCoreApplication::sendEvent(editSearch->nativeWidget(), endKeyEvent);

                endKeyEvent =
                    new QKeyEvent(QEvent::KeyRelease, Qt::Key_End,
                               Qt::NoModifier);
                QCoreApplication::sendEvent(editSearch->nativeWidget(), endKeyEvent);

                return true;
            }

            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_activeSection == "search") {
                    if (listSearchLeft->selectedIndex() == -1) {
                        listSearchLeft->initialSelection();
                    }

                    if (!editSearch->text().isEmpty()) {
                        m_completion->addItem(editSearch->text());
                        m_configUi.setSearchHistory(m_completion->items());
                    }
                }

                sendKeyEvent(keyEvent);
                break;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Menu:
                sendKeyEvent(keyEvent);
                break;

            case Qt::Key_Left:
                m_focusIndex--;
                break;

            case Qt::Key_Right:
                m_focusIndex++;
                break;

            case Qt::Key_PageUp:
                nextSection(-1);
                return true;

            case Qt::Key_PageDown:
                nextSection(1);
                return true;

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
                this, SLOT(lancelotToggleCentered())
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

void LancelotWindow::nextSection(int increment)
{
    kDebug() << increment;
    if (! (layoutMain->flip() & Plasma::VerticalFlip)) {
        increment = - increment;
    }

    int currentIndex = sectionIDs().indexOf(m_activeSection);
    currentIndex += increment;

    if (currentIndex >= sectionIDs().size()) {
        currentIndex = 0;
    } else if (currentIndex < 0) {
        currentIndex = sectionIDs().size() - 1;
    }

    sectionActivated(sectionIDs()[currentIndex]);
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
        Lancelot::Global::self()->group("SystemButtons")->setProperty("extenderPosition",
                (layoutMain->flip() & Plasma::VerticalFlip)
                    ? (Lancelot::TopExtender) : (Lancelot::BottomExtender)
        );
        Lancelot::Global::self()->group("SystemButtons")
            ->setProperty("activationMethod", Lancelot::ExtenderActivate);
    } else {
        Lancelot::Global::self()->group("SystemButtons")
            ->setProperty("extenderPosition", QVariant(Lancelot::NoExtender));
        Lancelot::Global::self()->group("SystemButtons")
            ->setProperty("activationMethod", Lancelot::ClickActivate);
    }

    Lancelot::Global::self()->group("LancelotContext")->setProperty("extenderPosition",
            Lancelot::Global::self()->group("SystemButtons")->property("extenderPosition"));
    Lancelot::Global::self()->group("LancelotContext")->setProperty("activationMethod",
            Lancelot::Global::self()->group("SystemButtons")->property("activationMethod"));
    Lancelot::Global::self()->group("SystemButtons")->notifyUpdated();
    Lancelot::Global::self()->group("LancelotContext")->notifyUpdated();

    if (sectionNoClick) {
        Lancelot::Global::self()->group("SectionButtons")
            ->setProperty("activationMethod", Lancelot::HoverActivate);
    } else {
        Lancelot::Global::self()->group("SectionButtons")
            ->setProperty("activationMethod", Lancelot::ClickActivate);
    }
    Lancelot::Global::self()->group("SectionButtons")->notifyUpdated();

    if (listsNoClick) {
        Lancelot::Global::self()->group("ActionListView-Left")
            ->setProperty("extenderPosition", Lancelot::LeftExtender);
        Lancelot::Global::self()->group("ActionListView-Right")
            ->setProperty("extenderPosition", Lancelot::RightExtender);
        Lancelot::Global::self()->group("PassagewayView")
            ->setProperty("activationMethod", Lancelot::ExtenderActivate);
        Lancelot::Global::self()->group("PopupList")
            ->setProperty("extenderPosition", Lancelot::RightExtender);
    } else {
        Lancelot::Global::self()->group("ActionListView-Left")
            ->setProperty("extenderPosition", Lancelot::NoExtender);
        Lancelot::Global::self()->group("ActionListView-Right")
            ->setProperty("extenderPosition", Lancelot::NoExtender);
        Lancelot::Global::self()->group("PassagewayView")
            ->setProperty("activationMethod", Lancelot::ClickActivate);
        Lancelot::Global::self()->group("PopupList")
            ->setProperty("extenderPosition", Lancelot::NoExtender);
    }
    Lancelot::Global::self()->group("ActionListView-Left")->notifyUpdated();
    Lancelot::Global::self()->group("ActionListView-Right")->notifyUpdated();
    Lancelot::Global::self()->group("PassagewayView")->notifyUpdated();

    // PassagewayView settings
    if (m_configUi.appbrowserPopupSubmenus()) {
        passagewayApplications->setColumnLimit(-1);
    } else if (m_configUi.appbrowserColumnLimitted()) {
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

        i.key()->setTitle(Lancelot::Models::SystemActions::self()->actionTitle(
                    i.value()));
        i.key()->setIcon(Lancelot::Models::SystemActions::self()->actionIcon(
                    i.value()));
    }

    // Logging
    Lancelot::Models::Logger::self()->setEnabled(m_configUi.enableUsageStatistics());

    // Keep open
    Lancelot::Models::ApplicationConnector::self()->setAutohideEnabled(!m_configUi.checkKeepOpen->isChecked());

    // Runners
    ((Lancelot::Models::Runner *) m_models["Runner"])->reloadConfiguration();
}

void LancelotWindow::lancelotContext()
{
    if (!menuLancelotContext) {
        menuLancelotContext = new Lancelot::PopupMenu();

        connect(
                menuLancelotContext->addAction(KIcon("kmenuedit"),
                    i18n("Menu Editor")), SIGNAL(triggered(bool)),
                this, SLOT(showMenuEditor()));

        connect(
                menuLancelotContext->addAction(KIcon("configure-shortcuts"),
                    i18n("Configure Shortcuts...")), SIGNAL(triggered(bool)),
                this, SLOT(configureShortcuts()));

        connect(
                menuLancelotContext->addAction(KIcon("configure"),
                    i18n("Configure Lancelot menu...")), SIGNAL(triggered(bool)),
                this, SLOT(configureMenu()));

        connect(
                menuLancelotContext->addAction(KIcon("lancelot"),
                    i18n("About Lancelot")), SIGNAL(triggered(bool)),
                this, SLOT(showAboutDialog()));
    }

    // menuLancelotContext->show();
    // menuLancelotContext->exec(QCursor::pos());

    menuLancelotContext->updateSize();

    QRect g = buttonLancelotContext->geometry().toRect();
    g.moveTopLeft(g.topLeft() + geometry().topLeft());

    if (layoutMain->flip() & Plasma::VerticalFlip) {
        menuLancelotContext->move(g.bottomLeft());
    } else {
        menuLancelotContext->move(g.topLeft()
            - QPoint(0, menuLancelotContext->geometry().height()));
    }

    menuLancelotContext->exec(menuLancelotContext->pos());
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
    connect(dialog, SIGNAL(applyClicked()), this, SLOT(saveConfig()));
    connect(dialog, SIGNAL(okClicked()),    this, SLOT(saveConfig()));
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

    if (updateWindowSize()) {
    //     m_corona->
    //         setSceneRect(QRectF(QPointF(), event->size()));

    //     m_root->
    //         setGeometry(QRect(QPoint(), event->size()));
    }
}

#include "LancelotWindow.moc"

