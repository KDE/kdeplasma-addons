/*
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
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

#include "postwidget.h"

#include <QGraphicsAnchorLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>

#include <KDateTime>
#include <KColorScheme>
#include <KTextBrowser>

#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/TextBrowser>
#include <Plasma/ScrollWidget>

PostWidget::PostWidget(QGraphicsWidget *parent)
    : Plasma::Frame(parent),
      m_tz(KTimeZone::utc()),
      m_colorScheme(0)
{
    m_author = new Plasma::Label(this);
    m_author->nativeWidget()->setWordWrap(false);
    m_picture = new Plasma::IconWidget(this);
    m_picture->setMinimumSize(m_picture->sizeFromIconSize(32));
    connect(m_picture, SIGNAL(clicked()), this, SLOT(askProfile()));
    m_from = new Plasma::Label(this);
    m_from->nativeWidget()->setWordWrap(false);
    m_text = new Plasma::TextBrowser(this);
    m_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_text->nativeWidget()->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_text->nativeWidget()->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_text->nativeWidget()->setCursor( Qt::ArrowCursor );
    m_replyIcon = new Plasma::IconWidget(this);
    m_replyIcon->setText("@");
    connect(m_replyIcon, SIGNAL(clicked()), this, SLOT(askReply()));
    m_forwardIcon = new Plasma::IconWidget(this);
    m_forwardIcon->setText("RT");
    connect(m_forwardIcon, SIGNAL(clicked()), this, SLOT(askForward()));

    //corners
    QGraphicsAnchorLayout *lay = new QGraphicsAnchorLayout(this);
    lay->setSpacing(4);
    lay->addCornerAnchors(lay, Qt::TopLeftCorner, m_picture, Qt::TopLeftCorner);
    lay->addCornerAnchors(m_picture, Qt::TopRightCorner, m_author, Qt::TopLeftCorner);
    lay->addCornerAnchors(m_author, Qt::BottomLeftCorner, m_from, Qt::TopLeftCorner);

    //vertical
    lay->addAnchor(m_from, Qt::AnchorBottom, m_text, Qt::AnchorTop);
    lay->addAnchor(lay, Qt::AnchorBottom, m_text, Qt::AnchorBottom);
    lay->addAnchor(lay, Qt::AnchorRight, m_text, Qt::AnchorRight);

    lay->addAnchor(m_picture, Qt::AnchorBottom, m_replyIcon, Qt::AnchorTop);
    lay->addAnchor(m_replyIcon, Qt::AnchorBottom, m_forwardIcon, Qt::AnchorTop);
    lay->addAnchor(lay, Qt::AnchorBottom, m_forwardIcon, Qt::AnchorBottom);

    //horizontal
    lay->addAnchor(m_picture, Qt::AnchorRight, m_text, Qt::AnchorLeft);

}

PostWidget::~PostWidget()
{
}

void PostWidget::setData(const Plasma::DataEngine::Data &data)
{
    m_author->setText(data["User"].toString());
    QLocale english(QLocale::English, QLocale::UnitedStates);
    QDateTime dt = english.toDateTime(data.value( "Date" ).toString(), "ddd MMM dd HH:mm:ss +0000 yyyy");
    m_from->setText(i18n("%1 from %2",timeDescription( dt ), data.value( "Source" ).toString()));

    QString status = data["Status"].toString();
    status.replace(QRegExp("((http|https)://[^\\s<>'\"]+[^!,\\.\\s<>'\"\\]])"), "<a href='\\1'>\\1</a>");

    m_text->setText(QString( "<p><font color='%1'>%2</font></p>" ).arg( m_colorScheme->foreground().color().name()).arg( status ));
}

void PostWidget::setPicture(const QPixmap &picture)
{
    m_picture->setIcon(picture);
}

void PostWidget::setColorScheme(KColorScheme *scheme)
{
    m_colorScheme = scheme;
}

void PostWidget::askReply()
{
    emit reply("@" + m_author->text() + " ");
}

void PostWidget::askForward()
{
    emit forward("RT @" + m_author->text() + " " + m_text->nativeWidget()->toPlainText());
}

void PostWidget::askProfile()
{
    emit openProfile(m_author->text());
}

void PostWidget::setActionsShown(bool show)
{
    m_replyIcon->setVisible(show);
    m_forwardIcon->setVisible(show);
}

QString PostWidget::timeDescription( const QDateTime &dt )
{
    int diff = dt.secsTo(KDateTime::currentDateTime(m_tz).dateTime());

    if (diff < 60) {
        return i18n("Less than a minute ago");
    } else if (diff < 60*60) {
        return i18np("1 minute ago", "%1 minutes ago", diff/60);
    } else if (diff < 2*60*60) {
        return i18n("Over an hour ago");
    } else if (diff < 24*60*60) {
        return i18np("1 hour ago", "%1 hours ago", diff/3600);
    }

    return dt.toString(Qt::LocaleDate);
}

#include <postwidget.moc>
