/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "previewdialog.h"

// KDE
#include <KVBox>
#include <KIcon>

// Qt
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>

// Plasma
#include <plasma/theme.h>
#include <plasma/widgets/label.h>

PreviewDialog::PreviewDialog(QWidget *parent) : Plasma::Dialog(parent),
        isMoving(false),
        isResizing(false)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);

    QVBoxLayout *d_lay = new QVBoxLayout(this);
    d_lay->setSpacing(1);
    d_lay->setMargin(0);

    b_widget = new KVBox();
    b_widget->setPalette(palette());
    QPalette p = b_widget->palette();
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    p.setColor(QPalette::Base, c);
    b_widget->setPalette(p);

    m_label = new QLabel(this);
    QPalette l_palette = m_label->palette();
    c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    l_palette.setColor(QPalette::Base, c);
    c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    l_palette.setColor(QPalette::WindowText, c);
    m_label->setPalette(l_palette);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mime_icon = new QLabel();
    mime_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QToolButton *remove_i = new QToolButton(this);
    remove_i->setAutoRaise(true);
    remove_i->setIcon(KIcon("user-trash"));
    remove_i->setToolTip(i18n("Close and remove the file"));

    QToolButton *close_i = new QToolButton(this);
    close_i->setAutoRaise(true);
    close_i->setIcon(KIcon("dialog-close"));

    QToolButton *run_i = new QToolButton(this);
    run_i->setAutoRaise(true);
    run_i->setIcon(KIcon("system-run"));
    run_i->setToolTip(i18n("Open with the correct application"));

    connect(close_i, SIGNAL(clicked()), this, SIGNAL(closeClicked()));
    connect(remove_i, SIGNAL(clicked()), this, SIGNAL(removeClicked()));
    connect(run_i, SIGNAL(clicked()), this, SIGNAL(runClicked()));

    QHBoxLayout *n_lay = new QHBoxLayout();
    n_lay->addWidget(mime_icon);
    n_lay->addWidget(m_label);
    n_lay->addWidget(run_i);
    n_lay->addWidget(remove_i);
    n_lay->addWidget(close_i);

    d_lay->addLayout(n_lay);
    d_lay->addWidget(b_widget);

    b_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));

    setResizeHandleCorners(Dialog::All);
}

void PreviewDialog::updateColors()
{
    QPalette l_palette = m_label->palette();
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    l_palette.setColor(QPalette::Base, c);
    c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    l_palette.setColor(QPalette::WindowText, c);
    m_label->setPalette(l_palette);

    QPalette p = b_widget->palette();
    c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    p.setColor(QPalette::Base, c);
    b_widget->setPalette(p);
}

void PreviewDialog::setMimeIcon(const QPixmap &icon)
{
    mime_icon->setPixmap(icon);
}

void PreviewDialog::setTitle(const QString &title)
{
    m_label->setText(title);
}

QWidget* PreviewDialog::baseWidget()
{
    return static_cast<QWidget*>(b_widget);
}

QLabel* PreviewDialog::titleLabel()
{
    return m_label;
}

void PreviewDialog::mousePressEvent(QMouseEvent *event)
{
    if (!inControlArea(event->pos())) {
        isMoving = true;
        startPos = event->pos();
    }
    Plasma::Dialog::mousePressEvent(event);
}

void PreviewDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (isMoving) {
        QPoint offset( event->pos().x()-startPos.x(), event->pos().y()-startPos.y());
        move(pos()+=offset);
    }

    Plasma::Dialog::mouseMoveEvent(event);
}

void PreviewDialog::mouseReleaseEvent(QMouseEvent *event)
{
    isMoving = false;
    Plasma::Dialog::mouseReleaseEvent(event);
}
