/***************************************************************************
 *   Copyright 2009-2011 by Sebastian Kugler <sebas@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "webslice.h"

#include <kgraphicswebslice.h>
#include <limits.h>

// Qt
#include <QGraphicsSceneResizeEvent>
#include <QLabel>

#include <QGraphicsLinearLayout>
#include <QWebElement>
#include <QWebFrame>

// KDE
#include <KDebug>
#include <KConfigDialog>
#include <KServiceTypeTrader>
#include <Plasma/Theme>

// Plasma
#include <Plasma/Label>

WebSlice::WebSlice(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
    m_slice(0),
    m_url(0)
{
    setPopupIcon("internet-web-browser");
    setAspectRatioMode(Plasma::IgnoreAspectRatio );
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    setMinimumSize(64, 64);
    resize(800, 600);
    kDebug() << "0";
}

void WebSlice::init()
{
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName());
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet",
                                                                    constraint);
    foreach (const KService::Ptr &service, offers) {
        QStringList args = service->property("X-Plasma-Args").toStringList();
        if (args.count() >= 1) {
            m_url = KUrl(args[0]);
            if (args.count() >= 2) {
                m_element = args[1];
                if (args.count() >= 6) {
                    m_sliceGeometry = QRectF(args[2].toInt(), args[3].toInt(),
                                            args[4].toInt(), args[5].toInt());
                }
            }
        }
    }
    void graphicsWidget();
}

WebSlice::~WebSlice ()
{
}

void WebSlice::configChanged()
{
    kDebug();
    KConfigGroup cg = config();
    if (!m_url.isValid() || m_url.isEmpty()) {
        m_url = cg.readEntry("url", "http://dot.kde.org/");
        m_element = cg.readEntry("element", "#block-views-planetkde-block_1");

        // for testing geometry
        //m_url = cg.readEntry("url", "http://buienradar.nl/");
        //m_sliceGeometry = cg.readEntry("size", QRectF(258, 102, 550, 511));
        m_sliceGeometry = cg.readEntry("sliceGeometry", QRectF());
    }
    setAssociatedApplicationUrls(KUrl::List(m_url));
    if (m_slice) {
        m_slice->preview();
    }
    loadSlice(m_url, m_element);
}

QGraphicsWidget* WebSlice::graphicsWidget()
{
    if (!m_slice) {
        m_widget = new QGraphicsWidget(this);
        QGraphicsLinearLayout *l = new QGraphicsLinearLayout(m_widget);
        m_widget->setLayout(l);


        m_slice = new KGraphicsWebSlice(m_widget);
        m_slice->setMaximumSize(contentsRect().size());
        //m_slice->show();
        updateColors();
        //connect(m_slice, SIGNAL(sizeChanged(QSizeF)), this, SLOT(sizeChanged(QSizeF)));
        connect(m_slice, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));

        setBusy(true);
        m_slice->setLoadingText(i18nc("displayed in the widget while loading", "<h1>Loading...</h1>"));
        //m_slice->hide();
        l->addItem(m_slice);
        kDebug() << "slice set up";
        configChanged();
    }
    return m_widget;
}

void WebSlice::createConfigurationInterface(KConfigDialog *parent)
{
    //QLabel *info = new QLabel(parent);
    QLabel* info = new QLabel(parent);
    info->setWordWrap(true);
    info->setTextInteractionFlags(Qt::TextBrowserInteraction);
    info->setText(i18n("<p>The Webslice Widget allows you to display a part of a webpage on your desktop or in a panel. The webslice is fully interactive.</p>"
    "<p>"
    "Specify the URL of the webpage in the URL field."
    "In the <em>Element to Show</em> field, fill in a CSS identifier (for example #mybox for elements with the id \"mybox\").  This is the preferred method as it works best with layout changes on the webpage."
    "</p>"
    "<p>"
    "Alternatively, you can specify a rectangle on the webpage to use as slice. Use \"x,y,width,height\" in pixels, for example <em>\"100,80,300,360\"</em>. This method is a fallback for webpages that do not provide enough semantic markup for the above mechanism."
    "</p>"
    "If both methods (element and geometry) are used, the element will take precedence."));
    parent->addPage(info, i18nc("informational page", "Info"), "help-hint");

    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18nc("general config page", "Webpage"), Applet::icon());
    connect(ui.loadUrl, SIGNAL(clicked()), this, SLOT(loadUrl()));
    connect(ui.elementCombo, SIGNAL(destroyed()), SLOT(disconnectLoadFinished()));
    connect(ui.elementCombo, SIGNAL(activated(QString)), this, SLOT(preview(QString)));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.urlEdit->setText(m_url.toString());
    ui.loadUrl->setIcon(KIcon("view-refresh"));
    ui.geometryEdit->setText(sliceGeometryToString());
    ui.elementCombo->setEditable(true);
    updateElements();
    preview(m_element);
    connect(ui.urlEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.loadUrl, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.elementCombo, SIGNAL(editTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.geometryEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(ui.elementCombo, SIGNAL(editTextChanged(QString)), this, SLOT(handleGeometryEdit()));
    connect(parent, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this,
            SLOT(handleGeometryEdit()));
}

void WebSlice::updateElements()
{
    ui.elementCombo->clear();
    ui.elementCombo->addItem(m_element, m_element);
    ui.elementCombo->addItem(QString("body"), QString("body"));
    foreach(const QWebElement el, m_slice->page()->mainFrame()->findAllElements("*")) {
        QString elSelector;
        QString elAttributeName;
        if (el.attributeNames().contains("id")) {
            elAttributeName = QString("id");
            elSelector = QString("#%1").arg(el.attribute("id")); // according to CSS selector syntax
        } else {
            /* don't add all elements for now, this really only clutters the combo
            elSelector = el.localName();
            if (!el.attribute("class").isEmpty()) {
                elSelector.append("." + el.attribute("class"));
            }
            */
        }
        // Add Item?
        if (!elSelector.isEmpty() && !(el.geometry().size().isNull())) {
            ui.elementCombo->addItem(elSelector, elAttributeName);
        }
    }
}

void WebSlice::disconnectLoadFinished()
{
    // we need to prevent the combo from being updated when it's gone
    disconnect(m_slice, SIGNAL(loadFinished(bool)), this, SLOT(updateElements()));
    m_slice->preview();
}

void WebSlice::preview(const QString &selector)
{
    ui.geometryEdit->setText(sliceGeometryToString(selector));

    m_slice->preview(selector);
}

void WebSlice::loadUrl()
{
    loadSlice(QUrl(ui.urlEdit->text()));
    connect(m_slice, SIGNAL(loadFinished(bool)), SLOT(updateElements()));
}

void WebSlice::loadSlice(const QUrl &url, const QString &selector)
{
    if (m_slice) {
        m_slice->loadSlice(url, selector);
        setAssociatedApplicationUrls(KUrl::List(url));
    }
}

void WebSlice::configAccepted()
{
    if (m_url.toString() != ui.urlEdit->text() ||
        m_element != ui.elementCombo->currentText() ||
        ui.geometryEdit->text() != sliceGeometryToString()) {

        m_url = QUrl(ui.urlEdit->text());
        m_element = ui.elementCombo->currentText();

        QString geo = ui.geometryEdit->text();
        QStringList gel = geo.split(',');
        qreal x, y, w, h;
        bool ok = true;
        if (gel.length() == 4) {
            x = (qreal)(gel[0].toDouble(&ok));
            y = (qreal)(gel[1].toDouble(&ok));
            w = (qreal)(gel[2].toDouble(&ok));
            h = (qreal)(gel[3].toDouble(&ok));
            if (!ok) {
                kWarning() << "a conversion error occurred." << gel;
            } else {
                m_sliceGeometry = QRectF(x, y, w, h);
                config().writeEntry("sliceGeometry", m_sliceGeometry);
            }
        } else {
            if (m_element.isEmpty()) {
                kWarning() << "format error, use x,y,w,h" << gel << gel.length();
            }
        }

        KConfigGroup cg = config();
        cg.writeEntry("url", m_url.toString());
        cg.writeEntry("element", m_element);
        if (!m_element.isEmpty()) {
            m_sliceGeometry = QRectF();
        }
        emit configNeedsSaving();
        configChanged();
    }
}

QString WebSlice::sliceGeometryToString(const QString &selector)
{
    QString s = QString("%1,%2,%3,%4").arg(m_slice->sliceGeometry(selector).x()).arg(m_slice->sliceGeometry(selector).y()).arg(m_slice->sliceGeometry(selector).width()).arg(m_slice->sliceGeometry(selector).height());
    return s;
}

void WebSlice::loadFinished(bool ok)
{
    setBusy(false);

    if (!ok) {
      return;
    }
}

void WebSlice::updateColors()
{
    m_slice->setPreviewMaskColor(
        Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
}

void WebSlice::handleGeometryEdit()
{
    if (ui.elementCombo->currentText().isEmpty()) {
        ui.geometryEdit->setEnabled(true);
    } else {
        ui.geometryEdit->setEnabled(false);
    }
}

#include "webslice.moc"
