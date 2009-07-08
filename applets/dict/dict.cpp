/*
 *   Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>
 *
 *   based on work by:
 *   Copyright (C) 2007 Thomas Georgiou <TAGeorgiou@gmail.com> and Jeff Cooper <weirdsox11@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dict.h"

#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KTextBrowser>
#include <KDebug>
#include <KIcon>
#include <KStandardDirs>
#include <KLineEdit>
#include <KEditListBox>
#include <QListView>
#include <QTreeView>
#include <QStringListModel>

#include <KColorScheme>
#include <KConfigDialog>
#include <KConfigGroup>

#include <Plasma/Animator>
#include <Plasma/IconWidget>
#include <Plasma/LineEdit>
#include <Plasma/TextBrowser>
#include <Plasma/Theme>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>


#define AUTO_DEFINE_TIMEOUT 500

using namespace Plasma;



// Style sheet format
const char* translationCSS =
    "dl {color: %1;}\n"
    "a{color: %2}\n"
    "a:visited{color: %3}\n";

DictApplet::DictApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , m_graphicsWidget(0)
    , m_wordEdit(0)
    , m_dictsModel(0)
      //m_flash(0)
{
    setPopupIcon("accessories-dictionary");
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void DictApplet::init()
{
    const char* dataEngines[]={"dict","qstardict"};
    bool engineChoice = dataEngine(dataEngines[1])->isValid();
//     bool engineChoice = false; //for testing
    m_dataEngine = dataEngines[int(engineChoice)];
    setHasConfigurationInterface(engineChoice);

    // tooltip stuff
    Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);
    toolTipData.setMainText(name());
    toolTipData.setImage(KIcon("accessories-dictionary"));

    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
}

DictApplet::~DictApplet()
{
    m_defBrowser->deleteLater();
}

QGraphicsWidget *DictApplet::graphicsWidget()
{
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

    m_wordEdit = new LineEdit(this);
    m_wordEdit->nativeWidget()->setClearButtonShown( true );
    m_wordEdit->nativeWidget()->setClickMessage(i18n("Enter word to define here"));
    m_wordEdit->show();
    Plasma::Animator::self()->animateItem(m_wordEdit, Plasma::Animator::AppearAnimation);

    // Gets the color scheme from default theme
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());

    m_defBrowser = new Plasma::TextBrowser();
    m_defBrowser->nativeWidget()->setNotifyClick(true);
    connect(m_defBrowser->nativeWidget(),SIGNAL(urlClick(QString)),this,SLOT(linkDefine(QString)));
    m_defBrowser->nativeWidget()->document()->setDefaultStyleSheet(QString(translationCSS)
                                                .arg(colorScheme.foreground().color().name())
                                                .arg(colorScheme.foreground(KColorScheme::LinkText).color().name())
                                                .arg(colorScheme.foreground(KColorScheme::VisitedText).color().name()));
    m_defBrowser->hide();

//  Icon in upper-left corner
    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("accessories-dictionary");

//  Position lineedits
    //const int wordEditOffset = 40;
    m_icon->setPos(12,3);
    //m_wordProxyWidget->setPos(15 + wordEditOffset,7);
    //m_wordProxyWidget->show();
    // TODO m_wordEdit->setDefaultTextColor(Plasma::Theme::self()->color(Plasma::Theme::TextColor));

//  Timer for auto-define
    m_timer = new QTimer(this);
    m_timer->setInterval(AUTO_DEFINE_TIMEOUT);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(define()));

    m_horLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_horLayout->addItem(m_icon);
    m_horLayout->addItem(m_wordEdit);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_horLayout);
    m_layout->addItem(m_defBrowser);

    m_source.clear();
    dataEngine(m_dataEngine)->connectSource(m_source, this);
    connect(m_wordEdit, SIGNAL(editingFinished()), this, SLOT(define()));
    connect(m_wordEdit->nativeWidget(), SIGNAL(textChanged(QString)), this, SLOT(autoDefine(QString)));

    dataEngine(m_dataEngine)->connectSource("list-dictionaries", this);

    //connect(m_defEdit, SIGNAL(linkActivated(const QString&)), this, SLOT(linkDefine(const QString&)));

//  This is the fix for links/selecting text
    //QGraphicsItem::GraphicsItemFlags flags = m_defEdit->flags();
    //flags ^= QGraphicsItem::ItemIsMovable;
   // m_defEdit->setFlags(flags);

    /*m_flash = new Plasma::Flash(this);
    m_flash->setColor(Qt::gray);
    QFont fnt = qApp->font();
    fnt.setBold(true);
    m_flash->setFont(fnt);
    m_flash->setPos(25,-10);
    m_flash->resize(QSize(200,20));*/



    KConfigGroup cg = config();
    m_dicts = cg.readEntry("KnownDictionaries", QStringList());
    QStringList activeDictNames = cg.readEntry("ActiveDictionaries", QStringList());
    for (QStringList::const_iterator i = m_dicts.constBegin(); i != m_dicts.constEnd(); ++i)
        m_activeDicts[*i]=activeDictNames.contains(*i);

    m_graphicsWidget = new QGraphicsWidget(this);
    m_graphicsWidget->setLayout(m_layout);
    m_graphicsWidget->setPreferredSize(500, 200);

    return m_graphicsWidget;
}


void DictApplet::linkDefine(const QString &text)
{
    kDebug() <<"ACTIVATED";
    m_wordEdit->setText(text);
    define();
}

void DictApplet::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source=="list-dictionaries")
    {
        QStringList newDicts=data["dictionaries"].toStringList();
        bool changed=false;
        for (QStringList::const_iterator i = newDicts.constBegin(); i != newDicts.constEnd(); ++i)
        {
            if (!m_dicts.contains(*i))
            {
                m_dicts<<*i;
                m_activeDicts[*i]=true;
                changed=true;
            }
        }
        QStringList::iterator it = m_dicts.begin();
        while (it != m_dicts.end())
        {
            if (!newDicts.contains(*it))
            {
                it=m_dicts.erase(it);
                changed=true;
            }
            else
                ++it;
        }
        if (changed)
            configAccepted();

    }
//     Q_UNUSED(source);
    /*if (m_flash) {
        m_flash->kill();
    }*/
    if (!m_source.isEmpty()) {
        m_defBrowser->show();
    }

    if (data.contains("text")) {
        m_defBrowser->nativeWidget()->setHtml(data[QString("text")].toString());
    }

    updateGeometry();
}

void DictApplet::define()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    QString newSource = m_wordEdit->text();
    QStringList dictsList;

    for (QStringList::const_iterator i = m_dicts.constBegin(); i != m_dicts.constEnd(); ++i) {
        if (m_activeDicts.contains(*i) && m_activeDicts.value(*i)) {
            dictsList << *i;
        }
    }

    if (!newSource.isEmpty() && !dictsList.isEmpty()) {
        newSource.prepend(dictsList.join(",")+':');
    }

    if (newSource == m_source) {
        return;
    }

    dataEngine(m_dataEngine)->disconnectSource(m_source, this);

    if (!newSource.isEmpty()) {
        //get new definition
        //m_flash->flash(i18n("Looking up ") + m_word);
        m_source = newSource;
        dataEngine(m_dataEngine)->connectSource(m_source, this);
    } else {
        //make the definition box disappear
        m_defBrowser->hide();
    }

    updateConstraints();
}

void DictApplet::autoDefine(const QString &word)
{
    Q_UNUSED(word)
    m_timer->start();
}


class CheckableStringListModel: public QStringListModel
{
public:
    CheckableStringListModel(QObject* parent, const QStringList& dicts, const QHash<QString,bool>& activeDicts_)
        : QStringListModel(parent)
        , activeDicts(activeDicts_)
    {
        setStringList(dicts);
    }
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
        Q_UNUSED(section)
        Q_UNUSED(orientation)

        if (role!=Qt::DisplayRole)
            return QVariant();
        return i18n("Dictionary");
    }
    Qt::DropActions supportedDropActions() const {return Qt::MoveAction;}
    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled;
    }
    bool setData (const QModelIndex& index, const QVariant& value, int role=Qt::EditRole)
    {
        if (role==Qt::CheckStateRole)
        {
            activeDicts[stringList().at(index.row())]=value.toInt()==Qt::Checked;
            return true;
        }
        else
            return QStringListModel::setData(index,value,role);
    }
    QVariant data(const QModelIndex& index, int role=Qt::EditRole) const
    {
        if (!index.isValid())
            return QVariant();

        if (role==Qt::CheckStateRole)
            return (  activeDicts.contains(stringList().at(index.row()))&&activeDicts.value(stringList().at(index.row()))  )?Qt::Checked:Qt::Unchecked;
        return QStringListModel::data(index,role);
    }

public:
    QHash<QString,bool> activeDicts;
};


void DictApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QTreeView* widget=new QTreeView(parent);
    widget->setDragEnabled(true);
    widget->setAcceptDrops(true);
    widget->setDragDropMode(QAbstractItemView::InternalMove);
    widget->setDropIndicatorShown(true);
    widget->setItemsExpandable(false);
    widget->setAllColumnsShowFocus(true);
    widget->setRootIsDecorated(false);

    delete m_dictsModel;
    m_dictsModel=new CheckableStringListModel(parent,m_dicts,m_activeDicts);
    widget->setModel(m_dictsModel);

    parent->addPage(widget, parent->windowTitle(), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void DictApplet::popupEvent(bool shown)
{
    kDebug() << shown;
    if (shown && m_wordEdit) {
        focusEditor();
    }
}

void DictApplet::configAccepted()
{
    if (m_dictsModel)
    {
        m_dicts=m_dictsModel->stringList();
        m_activeDicts=m_dictsModel->activeDicts;
    }
    KConfigGroup cg = config();
    cg.writeEntry("KnownDictionaries", m_dicts);

    QStringList activeDictNames;
    for (QStringList::const_iterator i = m_dicts.constBegin(); i != m_dicts.constEnd(); ++i)
        if (m_activeDicts.contains(*i) && m_activeDicts.value(*i))
            activeDictNames<<*i;

    cg.writeEntry("ActiveDictionaries", activeDictNames);

    define();
    emit configNeedsSaving();
}

void DictApplet::focusEditor()
{
    m_wordEdit->clearFocus();
    m_wordEdit->setFocus();
    m_wordEdit->nativeWidget()->clearFocus();
    m_wordEdit->nativeWidget()->setFocus();
}

#include "dict.moc"
