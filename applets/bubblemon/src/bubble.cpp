/*
*   Copyright 2009 by Trever Fischer <wm161@wm161.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "bubble.h"

#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtGui/QSortFilterProxyModel>

#include <Plasma/Theme>
#include <Plasma/Svg>
#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>

#include <KDE/KConfigDialog>

#include <KDebug>


K_EXPORT_PLASMA_APPLET(bubblemon, Bubble)

Bubble::Bubble(QObject *parent, const QVariantList &args)
    :  Plasma::Applet(parent, args),
       m_showText(false),
       m_showingText(false),
       m_animated(true),
       m_val(0),
       m_max(0),
       m_speed(1000),
       m_animID(-1),
       m_labelTransparency(0)
{
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath(Plasma::Theme::defaultTheme()->imagePath("bubblemon/bubble"));
    m_svg->setContainsMultipleImages(true);

    connect(m_svg, SIGNAL(repaintNeeded()), this, SLOT(repaintNeeded()));

    setAcceptsHoverEvents(true);
    resize(200, 200);
    setAspectRatioMode(Plasma::Square);
}

Bubble::~Bubble()
{
}

void
Bubble::repaintNeeded()
{
    update();
}

void
Bubble::reloadTheme()
{
    m_svg->setImagePath(Plasma::Theme::defaultTheme()->imagePath("bubblemon/bubble"));
}

void
Bubble::interpolateValue()
{
    update();
}

void
Bubble::init()
{
    m_sensorModel = new QStandardItemModel(this);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(reloadTheme()));
    
    m_animator = new QTimer(this);
    m_animator->setInterval(75);
    connect(m_animator, SIGNAL(timeout()), this, SLOT(moveBubbles()));
    
    KConfigGroup cg = config();
    m_animated = cg.readEntry("animated", true);
    m_showText = cg.readEntry("showText", false);
    showLabel(m_showText);
    m_speed = cg.readEntry("speed", 500);
    m_sensor = cg.readEntry("sensor", QString());
    if (m_sensor.isEmpty())
        setConfigurationRequired(true);
    
    if (m_animated)
        m_animator->start();
    else
        m_animator->stop();
    
    m_interpolator = new QTimeLine(m_speed*1.5, this);
    connect(m_interpolator, SIGNAL(frameChanged(int)), this, SLOT(interpolateValue()));
    
    m_engine = dataEngine("systemmonitor");
    if (!m_engine->isValid()) {
        setFailedToLaunch(true,
                           i18nc("@info:status The system monitor data engine could not be found or loaded",
                                  "Could not load the System Monitor data engine."));
    } else {
        connect(m_engine, SIGNAL(sourceAdded(const QString)), this, SLOT(connectSensor()));
    }
    m_engine->connectSource(m_sensor, this, m_speed);
}

void
Bubble::resizeEvent(QGraphicsSceneResizeEvent *evt)
{
    Plasma::Applet::resizeEvent(evt);
    m_svg->resize(evt->newSize());
    m_bubbleHeight = m_svg->elementSize("bubble").height();
}

void
Bubble::connectSensor()
{
    m_engine->connectSource(m_sensor, this, m_speed);
}

void
Bubble::disconnectSensor()
{
    m_engine->disconnectSource(m_sensor, this);
}

void
Bubble::reconnectSensor()
{
    disconnectSensor();
    connectSensor();
}

void
Bubble::constraintsEvent(Plasma::Constraints constraints)
{
    Plasma::Applet::constraintsEvent(constraints);
    
    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(NoBackground);
    }
}

void
Bubble::hoverEnterEvent(QGraphicsSceneHoverEvent *evt)
{
    Q_UNUSED(evt)
    showLabel(true);
}

void
Bubble::hoverLeaveEvent(QGraphicsSceneHoverEvent *evt)
{
    Q_UNUSED(evt)
    if (!m_showText)
        showLabel(false);
}

void
Bubble::updateLabelAnimation(qreal trans)
{
    if (trans == 1)
        m_animID = -1;
    if (!m_showingText) {
        m_labelTransparency = qMin(1 - trans, m_labelTransparency);
    } else {
        m_labelTransparency = trans;
    }
    m_labelTransparency = qMax(qreal(0.0), m_labelTransparency);
    update();
}

void
Bubble::showLabel(bool show)
{
    if (m_showingText == show)
        return;
    m_showingText = show;
    const int FadeInDuration = 150;
    if (m_animID != -1)
        Plasma::Animator::self()->stopCustomAnimation(m_animID);
    m_animID = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                          Plasma::Animator::EaseOutCurve, this,
                                                          "updateLabelAnimation");
}

void
Bubble::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    m_svg->paint(painter, m_svg->elementRect("background"), "background");
    if (m_max>0 && m_val>0) {
        QRect clipRect(contentsRect);
        float drawValue;
        if (m_animated && !shouldConserveResources())
            drawValue = m_interpolator->currentFrame();
        else
            drawValue = m_val;
        clipRect.setTop(contentsRect.height()-(contentsRect.height()*((float)drawValue/m_max)));
        QPainterPath clipPath;
        QPainterPath fillPath;
        QPainterPath bubblePath;
        fillPath.addEllipse(m_svg->elementRect("fill").adjusted(-5, 0, 5, 5));
        bubblePath.addEllipse(m_svg->elementRect("fill"));
        clipPath.addRect(clipRect);
        painter->setClipPath(clipPath.intersected(fillPath));
        m_svg->paint(painter, m_svg->elementRect("fill"), "fill");
        if (m_bubbles.size()>0 && m_animated && !shouldConserveResources()) {
            painter->setClipPath(clipPath.intersected(bubblePath));
            foreach(const QPoint& p, m_bubbles) {
                if (p.y()<contentsRect.bottom())
                    m_svg->paint(painter, p, "bubble");
            }
        }
        painter->setClipping(false);
    }
    m_svg->paint(painter, m_svg->elementRect("glass"), "glass");
    if (m_labelTransparency > 0)
        drawLabel(painter, option, contentsRect);
}

void
Bubble::drawLabel(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF &contentsRect)
{
    Q_UNUSED(option);
    QPointF center = contentsRect.center();
    QFont font = painter->font();
    QFont oldFont = font;
    font.setPointSize(font.pointSize()+1);
    QRectF labelRect;
    do {
        font.setPointSize(font.pointSize()-1);
        painter->setFont(font);
        labelRect = painter->boundingRect(contentsRect,
                                          Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignVCenter, m_label);
        labelRect.moveCenter(center);
        labelRect.adjust(-3, -3, 3, 3);
    } while (labelRect.width() > boundingRect().width() && font.pointSize()>1);
    if (font.pointSize()<=1)
        return;
    painter->setFont(font);
    QColor background = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    QColor fontColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    background.setAlphaF(m_labelTransparency);
    painter->setPen(background);
    background.setAlphaF(m_labelTransparency*0.5);
    painter->setBrush(background);
    
    fontColor.setAlphaF(m_labelTransparency);
    
    painter->drawRoundedRect(labelRect, 3, 3);
    painter->setPen(fontColor);
    painter->drawText(labelRect, Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignVCenter, m_label);
    painter->setFont(oldFont);
}

void
Bubble::moveBubbles()
{
    if (!boundingRect().isEmpty() && int(m_bubbleHeight * m_bubbles.size()) > 0 && m_max > 0 && m_animated && !shouldConserveResources()) {
        QRectF rect = boundingRect();
        QList<QPoint>::iterator i;
        bool needsUpdate = false;
        int maxHeight = rect.height()-(m_val/(float)m_max*rect.height()+m_bubbleHeight);
        for(i=m_bubbles.begin();i!=m_bubbles.end();++i) {
            (*i).setY((*i).y()-m_bubbleSpeed);
            if ((*i).y()<maxHeight-m_bubbleHeight) {
                (*i).setY(rect.bottom()+(qrand() % (int)( m_bubbleHeight*m_bubbles.size() ) ) );
                (*i).setX(qrand() % (int)rect.width());
                needsUpdate = true;
            }
            if ((*i).y()<rect.bottom() && (*i).y()>maxHeight)
                needsUpdate = true;
        }
        if (needsUpdate)
            update();
    }
}

QPainterPath
Bubble::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void
Bubble::dataUpdated(QString name, Plasma::DataEngine::Data data)
{
    Q_UNUSED(name);
    int prev = m_val;
    m_val = data["value"].toDouble();
    if (data["max"].toDouble()>0)
        m_max = data["max"].toDouble();
    else
        m_max = qMax(m_max, m_val);
    
    m_label = data["name"].toString();
    
    Plasma::ToolTipContent tip;
    tip.setMainText(data["name"].toString());
    if (data["units"].toString() == "%")
        tip.setSubText(i18nc("@info:status Value as displayed in a percentage format", "%1%", m_val));
    else
        tip.setSubText(i18nc("@info:status Value for non-percentage units (such as memory usage.)",
                              "%1%3/%2%3 (%4%)",
                              KGlobal::locale()->formatNumber(m_val),
                              KGlobal::locale()->formatNumber(m_max),
                              data["units"].toString(),
                              (int)((float)m_val/m_max*100)));
    
    QString section = m_sensor.section('/',0,0);
    KIcon tipIcon(icon());
    tip.setImage(tipIcon.pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(this, tip);
    
    if (m_animated && !shouldConserveResources()) {
        int bubbleCount;
        if (m_max>0)
            bubbleCount = ((float)m_val/(float)m_max)*20;
        else
            bubbleCount = 0;
        while(m_bubbles.size()<bubbleCount)
            m_bubbles.append(QPoint(0, 0));
        while(m_bubbles.size()>bubbleCount)
            m_bubbles.removeLast();
        m_bubbleSpeed = (boundingRect().height()/20)*((float)m_val/m_max)*3;
        
        m_interpolator->stop();
        m_interpolator->setFrameRange(prev, m_val);
        m_interpolator->start();
    }
    update();
}

QString
Bubble::icon() const
{
    QString section = m_sensor.section('/',0,0);
    if (section == "cpu")
        return "cpu";
    if (section == "mem")
        return "media-flash";
    if (section == "system")
        return "computer";
    if (section == "partitions" || section == "disk")
        return "drive-harddisk";
    if (section == "network")
        return "network-wired";
    if (section == "acpi")
        return "battery";
    if (section == "lmsensors")
        return "media-flash";
    return "utilities-system-monitor";
}

void
Bubble::createConfigurationInterface(KConfigDialog* dlg)
{
    QWidget *page = new QWidget(dlg);
    ui.setupUi(page);
    
    m_sensorModel->clear();
    QStandardItem *sensorItem;
    QModelIndex currentSensor;
    foreach(const QString &sensor, m_engine->sources()) {
        Plasma::DataEngine::Data sensorData = m_engine->query(sensor);
        sensorItem = new QStandardItem(sensorData["name"].toString());
        sensorItem->setData(sensor);
        sensorItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_sensorModel->appendRow(sensorItem);
        if (sensor == m_sensor)
            currentSensor = m_sensorModel->indexFromItem(sensorItem);
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(m_sensorModel);
    proxy->setSourceModel(m_sensorModel);
    ui.sensorView->setModel(proxy);

    if (currentSensor.isValid()) {
        ui.sensorView->selectionModel()->select(currentSensor, QItemSelectionModel::ClearAndSelect);
    }

    ui.searchBox->setProxy(proxy);

    dlg->addPage(page, i18nc("@title:group Title for the bubblemon settings page","General"), icon());
    connect(dlg, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(dlg, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.animateBubbles->setChecked(m_animated ? Qt::Checked : Qt::Unchecked);
    ui.showText->setChecked(m_showText ? Qt::Checked : Qt::Unchecked);
    ui.updateSpeed->setValue(m_speed);
}

void
Bubble::configAccepted()
{
    KConfigGroup cg = config();
    if (m_animated != ui.animateBubbles->isChecked()) {
        m_animated = !m_animated;
        cg.writeEntry("animated", m_animated);
        if (m_animated)
            m_animator->start();
        else
            m_animator->stop();
    }

    if (m_showText != ui.showText->isChecked()) {
        m_showText = ui.showText->isChecked();
        showLabel(m_showText);
        cg.writeEntry("showText", m_showText);
    }
    
    if (m_speed != ui.updateSpeed->value()) {
        m_speed = ui.updateSpeed->value();
        m_interpolator->setDuration(m_speed);
        cg.writeEntry("speed", m_speed);
        reconnectSensor();
    }

    QItemSelectionModel *selection = ui.sensorView->selectionModel();
    if (m_sensor != selection->currentIndex().data(Qt::UserRole+1)) {
        disconnectSensor();
        m_sensor = selection->currentIndex().data(Qt::UserRole+1).toString();
        cg.writeEntry("sensor", m_sensor);
        setConfigurationRequired(false);
        connectSensor();
    }
    

    emit configNeedsSaving();
}
