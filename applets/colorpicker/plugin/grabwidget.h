/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef GRABWIDGET_H
#define GRABWIDGET_H

#include <QObject>
#include <QColor>

class QWidget;

class Grabber : public QObject
{
    Q_OBJECT
public:
    ~Grabber() override;

    virtual void pick() = 0;

    QColor color() const {
        return m_color;
    }

Q_SIGNALS:
    void colorChanged();

protected:
    void setColor(const QColor &color);
    explicit Grabber(QObject *parent = nullptr);

private:
    QColor m_color;
};

class X11Grabber : public Grabber
{
    Q_OBJECT
public:
    explicit X11Grabber(QObject *parent = nullptr);
    ~X11Grabber() override;

    void pick() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *m_grabWidget;
};

class KWinWaylandGrabber : public Grabber
{
    Q_OBJECT
public:
    explicit KWinWaylandGrabber(QObject *parent = nullptr);
    ~KWinWaylandGrabber() override;

    void pick() override;
};

class GrabWidget : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QColor currentColor READ currentColor NOTIFY currentColorChanged)

public:
    explicit GrabWidget(QObject *parent = nullptr);
    ~GrabWidget() override;

    QColor currentColor() const;

    Q_INVOKABLE void pick();
    Q_INVOKABLE void copyToClipboard(const QString &text);

Q_SIGNALS:
    void currentColorChanged();

private:
    Grabber *m_grabber = nullptr;

};

#endif // GRABWIDGET_H
