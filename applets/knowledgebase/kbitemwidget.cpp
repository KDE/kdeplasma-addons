/***************************************************************************
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
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

// own
#include "kbitemwidget.h"
#include "kbitemtitle.h"

//Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsSceneMouseEvent>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KTextBrowser>

// Plasma
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Frame>
#include <Plasma/TextBrowser>
#include <Plasma/ScrollWidget>

using namespace Plasma;

KBItemWidget::KBItemWidget(QGraphicsWidget *parent)
    : Frame(parent),
      m_isHovered(false),
      m_detailsWidget(0)
{
    setAcceptHoverEvents(true);
    setMinimumHeight(40);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    QGraphicsLinearLayout *titleLayout = new QGraphicsLinearLayout;


    m_title = new KBItemTitle(this);
    connect(m_title, SIGNAL(clicked()), this, SLOT(toggleDetails()));
    m_openBrowser = new Plasma::IconWidget(this);
    m_openBrowser->setIcon("applications-internet");
    m_openBrowser->setVisible(false);
    m_openBrowser->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    connect(m_openBrowser, SIGNAL(clicked()), this, SLOT(openBrowser()));

    m_category = new Label;

    titleLayout->addItem(m_title);
    titleLayout->addItem(m_openBrowser);

    m_layout->addItem(titleLayout);
    m_layout->addItem(m_category);

}

KBItemWidget::~KBItemWidget()
{
}

void KBItemWidget::setAtticaData(const Plasma::DataEngine::Data &data)
{
    m_ocsData = data;
    m_title->setText(data["Name"].toString());
    m_category->setText(i18n("<i>Category: %1</i>", data.value("category").toString()));
}

void KBItemWidget::openBrowser()
{
    KToolInvocation::invokeBrowser(m_ocsData["DetailPage"].value<QUrl>().toString());
}

void KBItemWidget::openProfile()
{
    KToolInvocation::invokeBrowser("http://www.opendesktop.org/usermanager/search.php?username="+m_ocsData["User"].toString());
}

QGraphicsWidget *KBItemWidget::dragTitle() const
{
    return m_title;
}

void KBItemWidget::setDetailsShown(const bool show)
{
    if (show == (m_detailsWidget != 0)) {
        return;
    }

    if (show) {
        m_detailsWidget = new QGraphicsWidget(this);
        QGraphicsGridLayout *lay = new QGraphicsGridLayout(m_detailsWidget);

        Plasma::IconWidget *avatar = new Plasma::IconWidget(m_detailsWidget);
        lay->addItem(avatar, 0,0);
        if (!m_pixmap.isNull()) {
            avatar->setIcon(m_pixmap);
        } else {
            avatar->setIcon(KIcon("system-users"));
        }
        connect(avatar, SIGNAL(clicked()), this, SLOT(openProfile()));

        Plasma::TextBrowser *question = new Plasma::TextBrowser(m_detailsWidget);
        question->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        question->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        lay->addItem(question, 0, 1);
        question->setText(i18n("<body><b>Question:</b> %1</body>" ,m_ocsData["Description"].toString()));
        question->nativeWidget()->document()->setDefaultStyleSheet(Plasma::Theme::defaultTheme()->styleSheet());
        question->setText(i18n("<body><b>Question:</b> %1</body>" ,m_ocsData["Description"].toString()));
        question->nativeWidget()->setTextInteractionFlags(Qt::NoTextInteraction);

        Plasma::TextBrowser *answer = new Plasma::TextBrowser(m_detailsWidget);
        answer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        answer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        lay->addItem(answer, 1, 0, 1, 2);
        answer->nativeWidget()->document()->setDefaultStyleSheet(Plasma::Theme::defaultTheme()->styleSheet());
        QString answerText = m_ocsData["Answer"].toString();
        if (answerText.length() > 0) {
            answer->setText(i18n("<body><b>Answer:</b> %1</body>", answerText));
        } else {
            answer->setText(i18n("<body><b>Not answered yet</b> </body>"));
        }
        answer->nativeWidget()->setTextInteractionFlags(Qt::NoTextInteraction);

        m_layout->addItem(m_detailsWidget);
    } else {
        m_layout->removeItem(m_detailsWidget);
        m_detailsWidget->deleteLater();
        m_detailsWidget = 0;
    }

    emit detailsVisibilityChanged(this, show);
}

void KBItemWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
}

void KBItemWidget::toggleDetails()
{
    setDetailsShown(m_detailsWidget == 0);
}

Plasma::DataEngine::Data KBItemWidget::data()
{
    return m_ocsData;
}

void KBItemWidget::updateActions()
{
    m_openBrowser->setVisible(m_isHovered);
}

void KBItemWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = true;
    updateActions();
}

void KBItemWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = false;
    updateActions();
}

#include "kbitemwidget.moc"
