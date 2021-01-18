/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef GRABWIDGET_H
#define GRABWIDGET_H

#include <QColor>
#include <QObject>

class QWidget;

class Grabber : public QObject
{
    Q_OBJECT
public:
    ~Grabber() override;

    virtual void pick() = 0;

    QColor color() const
    {
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
