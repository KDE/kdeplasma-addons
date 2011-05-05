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
#include <QtCore/QPropertyAnimation>
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
       m_animated(true),
       m_val(0),
       m_max(0),
       m_speed(1000),
       m_bubbles(20),
       m_bubbleCount(0),
       m_labelTransparency(0),
       m_rebuildClip(true)
{
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath(Plasma::Theme::defaultTheme()->imagePath("bubblemon/bubble"));

    connect(m_svg, SIGNAL(repaintNeeded()), this, SLOT(repaintNeeded()));

    setAcceptsHoverEvents(true);
    setAspectRatioMode(Plasma::Square);
    setBackgroundHints(NoBackground);
    
    m_animation = new QPropertyAnimation(this, "labelTransparency", this);
    m_animation->setDuration(200);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
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
    m_rebuildClip = true;
    update();
}

void
Bubble::init()
{
    m_svg->resize(geometry().width(), geometry().height());

    m_sensorModel = new QStandardItemModel(this);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(reloadTheme()));

    m_animator = new QTimer(this);
    m_animator->setInterval(75);
    connect(m_animator, SIGNAL(timeout()), this, SLOT(moveBubbles()));

    m_interpolator = new QTimeLine(m_speed, this);
    connect(m_interpolator, SIGNAL(frameChanged(int)), this, SLOT(interpolateValue()));

    m_engine = dataEngine("systemmonitor");
    if (!m_engine->isValid()) {
        setFailedToLaunch(true,
                           i18nc("@info:status The system monitor data engine could not be found or loaded",
                                  "Could not load the System Monitor data engine."));
    } else {
        connect(m_engine, SIGNAL(sourceAdded(QString)), this, SLOT(connectSensor()));
    }

    configChanged();

    m_bubbleRect = m_svg->elementSize("bubble");
}

void
Bubble::resizeEvent(QGraphicsSceneResizeEvent *evt)
{
    Plasma::Applet::resizeEvent(evt);
    qreal size = qMin(contentsRect().size().width(), contentsRect().size().height());
    m_svg->resize(size, size);
    m_bubbleRect = m_svg->elementSize("bubble");
    m_rebuildClip = true;
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
        if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
            setPreferredSize(-1,-1);
        } else {
            setPreferredSize(150, 150);
        }
    }
    
    if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
        setMinimumSize(30,30);
    } else {
        setMinimumSize(0,0);
    }
}

void
Bubble::hoverEnterEvent(QGraphicsSceneHoverEvent *evt)
{
    Q_UNUSED(evt)
    if (m_showText)
        showLabel(true);
}

void
Bubble::hoverLeaveEvent(QGraphicsSceneHoverEvent *evt)
{
    Q_UNUSED(evt)
    if (m_showText)
        showLabel(false);
}

qreal
Bubble::labelTransparency() const
{
    return m_labelTransparency;
}

void
Bubble::setLabelTransparency(qreal trans)
{
    m_labelTransparency = trans;
    update();
}

void
Bubble::showLabel(bool show)
{
    if (!show)
        m_animation->setDirection(QAbstractAnimation::Backward);
    else
        m_animation->setDirection(QAbstractAnimation::Forward);
    m_animation->start();
}

void
Bubble::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    if (configurationRequired()) {
        return;
    }

    painter->save();
    painter->translate(contentsRect.topLeft());
    m_svg->paint(painter, m_svg->elementRect("background"), "background");
    
    if (m_max>0 && m_val>0) {
        float drawValue;
        if (m_animated && !shouldConserveResources())
            drawValue = m_interpolator->currentFrame();
        else
            drawValue = m_val;
        if (m_rebuildClip) {
            //Clipping the fill is easy. We just stop after some point.
            QRectF clipRect(contentsRect);
            clipRect.setTop(contentsRect.height()-(contentsRect.height()*((float)drawValue/m_max)));
            m_clip = clipRect;

            //To clip the individual bubbles, we first build a path of the whole bubble.
            //Then we take that path and subtract the empty portion.
            //This would be easier of QPainterPath could simply subtract primitives, but alas.
            QPainterPath bubbleClipPath;
            QPainterPath bubblePath;
            QPainterPath filledPath;
            QRectF unfilledRect(contentsRect);
            unfilledRect.setBottom(clipRect.top());
            bubblePath.addEllipse(m_svg->elementRect("fill"));
            filledPath.addRect(unfilledRect);

            bubbleClipPath = bubblePath - filledPath;

            m_bubbleClip = bubbleClipPath;
            m_rebuildClip = false;
        }
        painter->setClipRect(m_clip);
        m_svg->paint(painter, m_svg->elementRect("fill"), "fill");
        if (m_bubbleCount>0 && m_animated && !shouldConserveResources()) {
            painter->setClipPath(m_bubbleClip);
            for(int i = 0;i<m_bubbleCount;i++) {
                if (m_bubbles.at(i).y()+m_bubbleRect.height()>m_clip.top())
                    m_svg->paint(painter, QRectF(m_bubbles.at(i), m_bubbleRect), "bubble");
            }
        }
        painter->setClipping(false);
    }
    m_svg->paint(painter, m_svg->elementRect("glass"), "glass");
    if (m_labelTransparency > 0)
        drawLabel(painter, option, contentsRect);
    painter->restore();
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
    if (!boundingRect().isEmpty() && int(m_bubbleRect.height() * m_bubbleCount) > 0 && m_max > 0 && m_animated && !shouldConserveResources()) {
        QRectF rect = boundingRect();
        QVector<QPoint>::iterator i;
        bool needsUpdate = false;
        int maxHeight = rect.height()-(m_val/(float)m_max*rect.height()+m_bubbleRect.height());
        for(i=m_bubbles.begin();i!=m_bubbles.end();++i) {
            (*i).setY((*i).y()-m_bubbleSpeed);
            if ((*i).y()<maxHeight-m_bubbleRect.height()) {
                (*i).setY(rect.bottom()+(qrand() % (int)( m_bubbleRect.height()*m_bubbleCount ) ) );
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
    
    if(prev == m_val)
        return;
    
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
        m_bubbleCount = ((float)m_val/(float)m_max)*20;
        /*int bubbleCount;
        if (m_max>0)
            bubbleCount = ((float)m_val/(float)m_max)*20;
        else
            bubbleCount = 0;*/
        /*while(m_bubbles.size()<bubbleCount)
            m_bubbles.append(QPoint(0, 0));
        while(m_bubbles.size()>bubbleCount)
            m_bubbles.removeLast();*/
        m_bubbleSpeed = (boundingRect().height()/20)*((float)m_val/m_max)*3;
        
        m_interpolator->stop();
        m_interpolator->setFrameRange(prev, m_val);
        m_interpolator->start();
    }
    
    int lower = qMin(m_val, prev);
    int upper = qMax(m_val, prev);
    
    //formula taken fron Bubble::paintInterface
    QRect toUpdate(0, geometry().height()-(geometry().height()*((float)upper/m_max)),
                   geometry().width(), geometry().height()-(geometry().height()*((float)lower/m_max)) );

    m_rebuildClip = true;
    update(toUpdate);
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
        if (sensor == m_sensor) {
            currentSensor = m_sensorModel->indexFromItem(sensorItem);
        }
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(m_sensorModel);
    proxy->setSourceModel(m_sensorModel);
    ui.sensorView->setModel(proxy);

    if (currentSensor.isValid()) {
        ui.sensorView->selectionModel()->setCurrentIndex(currentSensor, QItemSelectionModel::ClearAndSelect);
        ui.sensorView->scrollTo(currentSensor, QAbstractItemView::PositionAtTop);
    }

    ui.searchBox->setProxy(proxy);

    dlg->addPage(page, i18nc("@title:group Title for the bubblemon settings page","General"), icon());
    connect(dlg, SIGNAL(applyClicked()), this, SLOT(writeConfig()));
    connect(dlg, SIGNAL(okClicked()), this, SLOT(writeConfig()));
    ui.animateBubbles->setChecked(m_animated ? Qt::Checked : Qt::Unchecked);
    ui.showText->setChecked(m_showText ? Qt::Checked : Qt::Unchecked);
    ui.updateSpeed->setValue(m_speed);
    connect(ui.updateSpeed, SIGNAL(valueChanged(int)), dlg, SLOT(settingsModified()));
    connect(ui.animateBubbles, SIGNAL(toggled(bool)), dlg, SLOT(settingsModified()));
    connect(ui.showText, SIGNAL(toggled(bool)), dlg, SLOT(settingsModified()));
    connect(ui.sensorView, SIGNAL(activated(QModelIndex)), dlg, SLOT(settingsModified()));
}

void
Bubble::writeConfig()
{
    KConfigGroup cg = config();
    bool changed = false;

    if (m_animated != ui.animateBubbles->isChecked()) {
        changed = true;
        cg.writeEntry("animated", ui.animateBubbles->isChecked());
    }

    if (m_showText != ui.showText->isChecked()) {
        changed = true;
        cg.writeEntry("showText", ui.showText->isChecked());
    }

    if (m_speed != ui.updateSpeed->value()) {
        changed = true;
        cg.writeEntry("speed", ui.updateSpeed->value());
    }

    QItemSelectionModel *selection = ui.sensorView->selectionModel();
    const QString sensor = selection->currentIndex().data(Qt::UserRole+1).toString();
    if (m_sensor != sensor) {
        changed = true;
        cg.writeEntry("sensor", sensor);
        setConfigurationRequired(false);
    }


    if (changed) {
        emit configNeedsSaving();
        m_rebuildClip = true;
    }
}

void
Bubble::configChanged()
{
    KConfigGroup cg = config();
    m_animated = cg.readEntry("animated", true);
    m_showText = cg.readEntry("showText", false);
    showLabel(m_showText);

    const int oldSpeed = m_speed;
    m_speed = cg.readEntry("speed", m_speed);
    m_interpolator->setDuration(m_speed);

    const QString sensor = cg.readEntry("sensor", m_sensor);
    if (m_sensor != sensor) {
        if (!m_sensor.isEmpty()) {
            disconnectSensor();
        }

        m_sensor = sensor;
        connectSensor();
    } else if (oldSpeed != m_speed && !m_sensor.isEmpty()) {
        reconnectSensor();
    }

    if (m_sensor.isEmpty())
        setConfigurationRequired(true);

    if (m_animated)
        m_animator->start();
    else
        m_animator->stop();

    update();
}


