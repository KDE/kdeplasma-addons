/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef GRABWIDGET_H
#define GRABWIDGET_H

#include <QColor>
#include <QObject>
#include <qqmlintegration.h>

class GrabWidget : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QColor currentColor READ currentColor NOTIFY currentColorChanged)

public:
    explicit GrabWidget(QObject *parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor &color);

    Q_INVOKABLE void pick();
    Q_INVOKABLE void copyToClipboard(const QString &text);

Q_SIGNALS:
    void currentColorChanged();

private:
    QColor m_currentColor;
};

#endif // GRABWIDGET_H
