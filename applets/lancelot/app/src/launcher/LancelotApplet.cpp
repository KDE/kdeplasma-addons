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

#include "LancelotApplet.h"
#include <KIcon>
#include <climits>

#include <QDBusInterface>
#include <QDBusReply>
#include <QGraphicsLinearLayout>

#include <lancelot/widgets/HoverIcon.h>

#include <lancelot/lancelot.h>
#include "lancelot_interface.h"
#include "../LancelotConfig.h"

#define SIZE_CMIN 16
#define SIZE_CPREF 32
#define SIZE_CMAX 64
#define SPACING 8

class LancelotApplet::Private {
public:
    Private(LancelotApplet * parent)
      : q(parent),
        layout(new QGraphicsLinearLayout(parent)),
        lancelot(NULL), offline(false)
    {
        q->setLayout(layout);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        lancelot = new org::kde::lancelot::App(
            "org.kde.lancelot", "/Lancelot",
            QDBusConnection::sessionBus()
        );
    }

    ~Private()
    {
        deleteButtons();
    }

    void deleteButtons()
    {
        while (layout->count()) {
            layout->removeAt(0);
        }

        foreach (Lancelot::HoverIcon * button, buttons) {
            delete button;
        }
        buttons.clear();
    }

    void createMainButton()
    {
        deleteButtons();
        Lancelot::HoverIcon * button = new Lancelot::HoverIcon(KIcon(mainIcon), "", q);
        layout->addItem(button);
        connect(button, SIGNAL(activated()), q, SLOT(showLancelot()));
        connect(button, SIGNAL(clicked()), q, SLOT(toggleLancelot()));
        button->setActivationMethod(clickActivation?(Lancelot::ClickActivate):(Lancelot::HoverActivate));

        buttons << button;
    }

    void createCategoriesButtons()
    {
        deleteButtons();

        // Loading Lancelot application categories
        QDBusReply<QStringList> replyIDs   = lancelot->sectionIDs();
        QDBusReply<QStringList> replyNames = lancelot->sectionNames();
        QDBusReply<QStringList> replyIcons = lancelot->sectionIcons();

        if (!replyIDs.isValid() || !replyNames.isValid() || !replyIcons.isValid()) {
            // Error connecting to Lancelot via d-bus
            // setFailedToLaunch(true);
            return;
        }

        // Creating buttons...
        for (int i = 0; i < replyIDs.value().size(); i++) {
            if (categsHide.contains(replyIDs.value().at(i))) {
                continue;
            }

            Lancelot::HoverIcon * button = new Lancelot::HoverIcon(
                KIcon(replyIcons.value().at(i)), "", q);

            connect(
                button, SIGNAL(activated()),
                & signalMapper, SLOT(map())
            );

            signalMapper.setMapping(button, replyIDs.value().at(i));

            connect(
                button, SIGNAL(clicked()),
                & signalMapperToggle, SLOT(map())
            );

            signalMapperToggle.setMapping(button, replyIDs.value().at(i));

            layout->addItem(button);
            button->setActivationMethod(clickActivation?(Lancelot::ClickActivate):(Lancelot::HoverActivate));

            buttons << button;
        }
    }

    void toggleHide()
    {
        if (waitClick.isActive()) {
            waitClick.stop();
        } else {
            lancelot->hide(true);
            offline = true;
        }
    }

    bool showingCategories;
    bool showCategories;
    QString mainIcon;
    bool clickActivation;
    QStringList categsHide;

    QSignalMapper signalMapper;
    QSignalMapper signalMapperToggle;
    LancelotApplet * q;
    QList < Lancelot::HoverIcon * > buttons;
    QGraphicsLinearLayout * layout;
    org::kde::lancelot::App * lancelot;
    QList < QAction * > actions;

    bool offline;
    QTimer waitClick;
};

LancelotApplet::LancelotApplet(QObject * parent,
        const QVariantList &args):
    Plasma::Applet(parent, args), d(new Private(this))
{
    setHasConfigurationInterface(true);
    setBackgroundHints(NoBackground);
    setAcceptsHoverEvents(true);

    QAction * action;
    d->actions.append(action = new QAction(
            KIcon("configure-shortcuts"),
            i18n("Configure Shortcuts..."),
            parent));
    connect(action, SIGNAL(triggered(bool)), d->lancelot, SLOT(configureShortcuts()));

    d->actions.append(action = new QAction(
            KIcon(),
            i18n("Menu Editor"),
            parent));
    connect(action, SIGNAL(triggered(bool)), d->lancelot, SLOT(showMenuEditor()));

    d->waitClick.setInterval(500); // 1/2 sec
    d->waitClick.setSingleShot(true);
}

// void LancelotApplet::paint(QPainter * p,
//     const QStyleOptionGraphicsItem * options, QWidget * widget)
// {
//     p->fillRect(options->rect, QBrush(QColor()));
// }

LancelotApplet::~LancelotApplet()
{
    delete d;
}

void LancelotApplet::loadConfig()
{
    KConfigGroup kcg = config();
    d->showCategories =  (kcg.readEntry("show", "main") != "main");
    d->mainIcon = kcg.readEntry("icon", "lancelot");
    d->clickActivation = (kcg.readEntry("activate", "click") == "click");
    d->categsHide = kcg.readEntry("hiddenCategories", QStringList());
}

void LancelotApplet::saveConfig()
{
    KConfigGroup kcg = config();
    kcg.writeEntry("show", (d->showCategories?"categories":"main"));
    kcg.writeEntry("icon", d->mainIcon);
    kcg.writeEntry("activate", (d->clickActivation?"click":"hover"));
    kcg.writeEntry("hiddenCategories", d->categsHide);
    save(kcg);

    m_configMenu.saveConfig();
}

void LancelotApplet::applyConfig()
{
    d->layout->setContentsMargins(0, 0, 0, 0);
    d->layout->setSpacing(SPACING);
    if (d->showCategories) {
        d->createCategoriesButtons();
    } else {
        d->createMainButton();
    }
    emit configNeedsSaving();
    update();
    setAspectRatioMode(Plasma::KeepAspectRatio);
}

void LancelotApplet::init()
{
    d->lancelot->addClient();
    setAcceptsHoverEvents(true);
    loadConfig();
    applyConfig();
    connect(
        & d->signalMapper, SIGNAL(mapped(const QString &)),
        this, SLOT(showLancelotSection(const QString &))
    );
    connect(
        & d->signalMapperToggle, SIGNAL(mapped(const QString &)),
        this, SLOT(toggleLancelotSection(const QString &))
    );
    KGlobal::locale()->insertCatalog("lancelot");
}

void LancelotApplet::showLancelot()
{
    if (d->offline) return;

    if (!d->lancelot->isShowing()) {
        d->waitClick.start();
    }
    QPoint position = popupPosition(QSize());
    d->lancelot->show(position.x(), position.y());
}

void LancelotApplet::toggleLancelot()
{
    // If lancelot is not shown, we are going to show it
    // If it is shown, then we try to detect the accidental
    // toggle invocation - if the menu was not shown for
    // at least a second, then we suppose it is an accident.
    // Nevertheless, we stop the timer if it was accidental
    // because it is less likely that two accidental invocations
    // would come in a row
    if (d->lancelot->isShowing()) {
        d->toggleHide();
    } else {
        d->offline = false;
        showLancelot();
    }
}

void LancelotApplet::showLancelotSection(const QString & section)
{
    if (d->offline) return;

    if (!d->lancelot->isShowing()) {
        d->waitClick.start();
    }
    QPoint position = popupPosition(QSize());
    d->lancelot->showItem(position.x(), position.y(), section);
}

void LancelotApplet::toggleLancelotSection(const QString & section)
{
    if (d->lancelot->isShowing()) {
        d->toggleHide();
    } else {
        d->offline = false;
        showLancelotSection(section);
    }
}

void LancelotApplet::configAccepted()
{
    d->showCategories = m_config.showCategories();
    d->mainIcon = m_config.icon();
    d->clickActivation = m_config.clickActivation();
    d->categsHide = m_config.showingCategories(false);
    applyConfig();
    saveConfig();
    d->lancelot->configurationChanged();
}

void LancelotApplet::createConfigurationInterface(KConfigDialog * parent)
{
    QWidget * appletConfig = new QWidget(parent);
    m_config.setupUi(appletConfig);

    m_config.setShowCategories(d->showCategories);
    m_config.setIcon(d->mainIcon);
    m_config.setClickActivation(d->clickActivation);
    m_config.setShowingCategories(d->categsHide, false);
    parent->addPage(appletConfig, i18n("Applet"),
            "application-x-plasma", i18n("Lancelot Launcher Applet"));

    QWidget * menuConfig = new QWidget(parent);
    m_configMenu.setupUi(menuConfig);
    m_configMenu.loadConfig();
    parent->addPage(menuConfig, i18n("Menu"),
            "lancelot", i18n("Lancelot Menu"));

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void LancelotApplet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::LocationConstraint) {
        if (formFactor() == Plasma::Vertical) {
            d->layout->setOrientation(Qt::Vertical);
        } else {
            d->layout->setOrientation(Qt::Horizontal);
        }
    }

    if (formFactor() == Plasma::Horizontal) {
        setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    } else if (formFactor() == Plasma::Vertical) {
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    }
}

void LancelotApplet::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    Plasma::Applet::hoverLeaveEvent(event);
    d->offline = false;
}

QList< QAction * > LancelotApplet::contextualActions()
{
    d->offline = true;
    d->lancelot->hide(true);
    QList < QAction * > result = Plasma::Applet::contextualActions();
    result << d->actions;
    return result;
}

QSizeF LancelotApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = Plasma::Applet::sizeHint(which, constraint);
    if (formFactor() == Plasma::Horizontal &&
            (which == Qt::MaximumSize || size().height() <= KIconLoader::SizeLarge)) {
        hint.setWidth(size().height() * d->buttons.size());
    } else if (formFactor() == Plasma::Vertical &&
            (which == Qt::MaximumSize || size().width() <= KIconLoader::SizeLarge)) {
        hint.setHeight(size().width() * d->buttons.size());
    }

    return hint;
}

#include "LancelotApplet.moc"

