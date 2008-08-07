/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotPart.h"
#include "KDebug"
#include "KIcon"
#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <QDataStream>

#include "../src/models/Places.h"
#include "../src/models/FavoriteApplications.h"
#include "../src/models/Devices.h"

class FullLayout: public QGraphicsLayout {
public:
    FullLayout(QGraphicsLayoutItem * parent = 0)
        : QGraphicsLayout(parent)
    {
    }

    virtual int count() const
    {
        return m_items.size();
    }

    void setGeometry(const QRectF & rect)
    {
        QGraphicsLayout::setGeometry(rect);
        invalidate();
    }

    virtual void invalidate()
    {
        foreach (QGraphicsLayoutItem * item, m_items) {
            item->setGeometry(geometry());
            item->setPreferredSize(geometry().size());
        }
    }

    virtual QGraphicsLayoutItem * itemAt(int i) const
    {
        return m_items.at(i);
    }

    virtual void removeAt(int i)
    {
        return m_items.removeAt(i);
    }

    void addItem(QGraphicsLayoutItem * item)
    {
        m_items.append(item);
    }

    virtual QSizeF sizeHint(Qt::SizeHint hint, const QSizeF & constraint) const
    {
        QSizeF size;
        foreach (QGraphicsLayoutItem * item, m_items) {
            size.expandedTo(item->effectiveSizeHint(hint, constraint));
        }
        return size;
    }
private:
    QList< QGraphicsLayoutItem * > m_items;
};

LancelotPart::LancelotPart(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args), m_instance(NULL), m_list(NULL),
    m_model(NULL), m_cmdarg(QString())
{
    m_instance = new Lancelot::Instance();
    FullLayout * fullLayout = new FullLayout(this);
    m_layout = fullLayout;

    m_list = new Lancelot::ActionListView(this);
    fullLayout->addItem(m_list);
    setMinimumSize(QSizeF(150, 200));

    setLayout(m_layout);

    m_instance->activateAll();
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    if (args.size() > 0) {
        m_cmdarg = args[0].toString();
    }
}

void LancelotPart::init()
{
    resize(200, 300);
    bool loaded;
    loaded = loadConfig();
    if (!loaded) {
        loaded = load(m_cmdarg);
    }

    if (!loaded) {
    }
}

bool LancelotPart::load(const QString & input)
{
    QStringList lines = input.split("\n");
    QMap < QString, QString > data;

    kDebug() << input;

    foreach (QString line, lines) {
        if (line.isEmpty()) continue;
        QStringList lineParts = line.split("=");
        data[lineParts[0]] = lineParts[1];
    }

    kDebug() << data;

    if (!data.contains("version")) {
        return false;
    }

    bool loaded = false;

    if (data["version"] <= "1.0") {
        if (data["type"] == "list") {
            QString modelID = data["model"];
            if (m_model && m_deleteModel) {
                delete m_model;
            }
            if (modelID == "FavoriteApplications") {
                m_model = Models::FavoriteApplications::instance();
                m_deleteModel = false;
                loaded = true;
            } else if (modelID == "Devices") {
                m_model = new Models::Devices();
                m_deleteModel = true;
                loaded = true;
            } else if (modelID == "Places") {
                m_model = new Models::Places();
                m_deleteModel = true;
                loaded = true;
            }
            m_list->setModel(m_model);
        }
    }

    return loaded;
}

LancelotPart::~LancelotPart()
{
    m_layout->removeAt(0);
    delete m_list;
    delete m_instance;
}

void LancelotPart::saveConfig(const QString & data)
{
    KConfigGroup kcg = config();
    kcg.writeEntry("partData", data);
}

bool LancelotPart::loadConfig()
{
    KConfigGroup kcg = config();
    kDebug() << kcg.entryMap();

    QString data = kcg.readEntry("partData", QString());
    if (data.isEmpty()) {
        return false;
    }
    return load(data);
}

//#include "LancelotPart.maoc"
