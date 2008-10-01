/*
 *   Copyright (C) 2007, 2008 Petri Damsten <damu@iki.fi>
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

#include "lcd.h"
#include <KDebug>
#include <KFilterDev>
#include <KSvgRenderer>
#include <QPainter>
#include <QDir>
#include <QGraphicsLinearLayout>
#include <QDomDocument>
#include <Plasma/Theme>

class LCD::Private
{
    public:
        LCD* l;
        QString content;
        // We don't use Plasma::Svg here because it has some accuracy problems.
        // lcd numbers did not look good with that.
        KSvgRenderer svg;
        QStringList items;
        bool dirty;
        QImage img;
        QMap<QString, QStringList> groups;

        static const QString A;
        static const QString B;
        static const QString C;
        static const QString D;
        static const QString E;
        static const QString F;
        static const QString G;
        static const QString DP;
        static QMap<QChar, QStringList> sevenSegmentDigits;

        Private(LCD* lcd) : l(lcd), dirty(false)
        {
            // http://en.wikipedia.org/wiki/Seven-segment_display_character_representations
            if (sevenSegmentDigits.isEmpty()) {
                sevenSegmentDigits[' '] = QStringList();
                sevenSegmentDigits['-'] = QStringList() << G;
                sevenSegmentDigits['0'] = QStringList() << A << B << C << D << E << F;
                sevenSegmentDigits['1'] = QStringList() << B << C;
                sevenSegmentDigits['2'] = QStringList() << A << B << D << E << G;
                sevenSegmentDigits['3'] = QStringList() << A << B << C << D << G;
                sevenSegmentDigits['4'] = QStringList() << B << C << F << G;
                sevenSegmentDigits['5'] = QStringList() << A << C << D << F << G;
                sevenSegmentDigits['6'] = QStringList() << A << C << D << E << F << G;
                sevenSegmentDigits['7'] = QStringList() << A << B << C;
                sevenSegmentDigits['8'] = QStringList() << A << B << C << D << E << F << G;
                sevenSegmentDigits['9'] = QStringList() << A << B << C << D << F << G;
                sevenSegmentDigits['A'] = QStringList() << A << B << C << E << F << G;
                sevenSegmentDigits['B'] = QStringList() << A << B << C << D << E << F << G;
                sevenSegmentDigits['C'] = QStringList() << A << D << E << F;
                sevenSegmentDigits['D'] = QStringList() << A << B << C << D << E << F;
                sevenSegmentDigits['E'] = QStringList() << A << D << E << F << G;
                sevenSegmentDigits['F'] = QStringList() << A << E << F << G;
                sevenSegmentDigits['G'] = QStringList() << A << C << D << E << F;
                sevenSegmentDigits['H'] = QStringList() << B << C << E << F << G;
                sevenSegmentDigits['I'] = QStringList() << B << C;
                sevenSegmentDigits['J'] = QStringList() << B << C << D << E;
                sevenSegmentDigits['K'] = QStringList() << E << F << G;
                sevenSegmentDigits['L'] = QStringList() << D << E << F;
                sevenSegmentDigits['M'] = QStringList() << A << B << D << F;
                sevenSegmentDigits['N'] = QStringList() << A << B << C << E << F;
                sevenSegmentDigits['O'] = QStringList() << A << B << C << D << E << F;
                sevenSegmentDigits['P'] = QStringList() << A << B << E << F << G;
                sevenSegmentDigits['Q'] = QStringList() << A << B << D << F << G;
                sevenSegmentDigits['R'] = QStringList() << A << E << F;
                sevenSegmentDigits['S'] = QStringList() << A << C << D << F << G;
                sevenSegmentDigits['T'] = QStringList() << A << E << F;
                sevenSegmentDigits['U'] = QStringList() << B << C << D << E << F;
                sevenSegmentDigits['V'] = QStringList() << B << C << D << E << F;
                sevenSegmentDigits['W'] = QStringList() << B << D << F << G;
                sevenSegmentDigits['X'] = QStringList() << B << C << E << F << G;
                sevenSegmentDigits['Y'] = QStringList() << B << C << F << G;
                sevenSegmentDigits['Z'] = QStringList() << A << B << D << E << G;
                sevenSegmentDigits['a'] = QStringList() << A << B << C << D << E << G;
                sevenSegmentDigits['b'] = QStringList() << C << D << E << F << G;
                sevenSegmentDigits['c'] = QStringList() << D << E << G;
                sevenSegmentDigits['d'] = QStringList() << B << C << D << E << G;
                sevenSegmentDigits['e'] = QStringList() << A << B << E << F << G;
                sevenSegmentDigits['f'] = QStringList() << A << E << F << G;
                sevenSegmentDigits['g'] = QStringList() << A << B << C << D << F << G;
                sevenSegmentDigits['h'] = QStringList() << C << E << F << G;
                sevenSegmentDigits['i'] = QStringList() << C;
                sevenSegmentDigits['j'] = QStringList() << C << D;
                sevenSegmentDigits['k'] = QStringList() << A << C << E << F << G;
                sevenSegmentDigits['l'] = QStringList() << E << F;
                sevenSegmentDigits['m'] = QStringList() << A << C << E << G;
                sevenSegmentDigits['n'] = QStringList() << C << E << G;
                sevenSegmentDigits[QChar(0x00F1)] = QStringList() << A << C << E << G; // ñ
                sevenSegmentDigits['o'] = QStringList() << C << D << E << G;
                sevenSegmentDigits['p'] = QStringList() << A << B << E << F << G;
                sevenSegmentDigits['q'] = QStringList() << A << B << C << F << G;
                sevenSegmentDigits['r'] = QStringList() << E << G;
                sevenSegmentDigits['s'] = QStringList() << E << G;
                sevenSegmentDigits['t'] = QStringList() << D << E << F << G;
                sevenSegmentDigits['u'] = QStringList() << C << D << E;
                sevenSegmentDigits['v'] = QStringList() << E << G;
                sevenSegmentDigits['w'] = QStringList() << E << G;
                sevenSegmentDigits['x'] = QStringList() << E << G;
                sevenSegmentDigits['y'] = QStringList() << E << G;
                sevenSegmentDigits['z'] = QStringList() << E << G;
                sevenSegmentDigits[QChar(0x00F6)] = QStringList() << A << C << D << E << G; // ö
            }
        }

        int digits(const QString& name)
        {
            return groups[name].count();
        }

        void paint(QPainter* p, const QString& item)
        {
            QRectF r = svg.boundsOnElement(item);
            svg.render(p, item, r);
        }

        void updateImage()
        {
            if (l->size().toSize() != img.size()) {
                img = QImage(l->size().toSize(), QImage::Format_ARGB32);
            }
            img.fill(Qt::transparent);

            QPainter p(&img);

            p.scale(l->size().width() / svg.defaultSize().width(),
                    l->size().height() / svg.defaultSize().height());

            paint(&p, "background");
            foreach (const QString& item, items) {
                paint(&p, item);
            }
            dirty = false;
        }

        void parseXml()
        {
            QIODevice *device = KFilterDev::deviceForFile(content);
            QDomDocument doc;

            doc.setContent(device);
            QList<QDomNodeList> lists;
            int pos;
            lists << doc.elementsByTagName("g");
            lists << doc.elementsByTagName("path");
            foreach (const QDomNodeList& list, lists) {
                for (int i = 0; i < list.count(); ++i) {
                    QDomElement element = list.item(i).toElement();
                    QString id = element.attribute("id");
                    if ((pos = id.lastIndexOf(':')) > -1) {
                        groups[id.left(pos)] << id.mid(pos + 1);
                    }
                }
            }
            //kDebug() << groups;
            delete device;
        }

};

QMap<QChar, QStringList> LCD::Private::sevenSegmentDigits;
const QString LCD::Private::A("A");
const QString LCD::Private::B("B");
const QString LCD::Private::C("C");
const QString LCD::Private::D("D");
const QString LCD::Private::E("E");
const QString LCD::Private::F("F");
const QString LCD::Private::G("G");
const QString LCD::Private::DP("DP");

LCD::LCD(QGraphicsItem *parent) :
        QGraphicsWidget(parent),
        d(new Private(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

LCD::~LCD()
{
    delete d;
}

QString LCD::svg() const
{
    return d->content;
}

void LCD::setSvg(const QString &svg)
{
    if (QDir::isAbsolutePath(svg)) {
        d->content = svg;
    } else {
        d->content = Plasma::Theme::defaultTheme()->imagePath(svg);
    }
    d->svg.load(d->content);
    d->parseXml();
    setPreferredSize(d->svg.defaultSize());
    d->dirty = true;
    update();
}

QStringList LCD::groupItems(const QString &name)
{
    return d->groups[name];
}

void LCD::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (d->dirty || size().toSize() != d->img.size()) {
        d->updateImage();
    }
    p->drawImage(0, 0, d->img);
}

void LCD::setDigit(const QString &name, QChar digit, bool dot)
{
    QStringList segments;

    //kDebug() << name << digit << dot;
    if (d->sevenSegmentDigits.keys().contains(digit)) {
        segments = d->sevenSegmentDigits[digit];
    }
    if (dot) {
        segments << d->DP;
    }
    setGroup(name, segments);
}

void LCD::setNumber(const QString &name, const QString& number)
{
    int j = 0;
    int digits = d->digits(name);
    bool dot = false;

    //kDebug() << name << number << digits;
    for (int i = number.length() - 1; i >= 0; --i) {
        if (number[i] == '.') {
            dot = true;
        } else {
            setDigit(name + QString(":%1").arg(j++), number[i], dot);
            dot = false;
        }
        if (j >= digits) {
            break;
        }
    }
    for (int i = j; i < digits; ++i) {
        setDigit(name + QString(":%1").arg(i), ' ', false);
    }
}

void LCD::setGroup(const QString &name, const QStringList& on)
{
    QStringList all = d->groups[name];
    foreach (const QString& item, all) {
        if (on.contains(item)) {
            setItemOn(name + ':' + item);
        } else {
            setItemOff(name + ':' + item);
        }
    }
}

void LCD::setItemOn(const QString &name)
{
    if (!d->items.contains(name)) {
        //kDebug() << "++++++++++" << name;
        d->items << name;
        d->dirty = true;
        update();
    }
}

void LCD::setItemOff(const QString &name)
{
    if (d->items.contains(name)) {
        //kDebug() << "----------" << name;
        d->items.removeAll(name);
        d->dirty = true;
        update();
    }
}

#include "lcd.moc"
