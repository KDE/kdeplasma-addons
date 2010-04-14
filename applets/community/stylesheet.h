/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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

#ifndef STYLESHEET_H
#define STYLESHEET_H

//Qt
#include <QtCore/QHash>

// KDE
#include <KDirWatch>

class StyleSheet : public QObject
{
    Q_OBJECT

    public:
        StyleSheet(QObject *parent);
        virtual ~StyleSheet();

        void setFileName(const QString &cssFile);

        QString styleSheet() const;
        void setStyleSheet(const QString &css);

    Q_SIGNALS:
        void styleSheetChanged(const QString&);

    public Q_SLOTS:
        void load(const QString &cssFile);
        void update();

    private:
        QString m_cssFile;
        QString m_styleSheet;
        QString m_rawStyleSheet;

        QHash<QString, QString> m_colors;
        KDirWatch* m_cssWatch;
};

#endif

