/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dockconfig.h"
#include "dockmanager.h"
#include <KDE/KConfigDialog>
#include <KDE/KStandardDirs>
#include <KDE/KConfigGroup>
#include <KDE/KConfig>
#include <KDE/KLocale>
#include <KDE/KIcon>
#include <KDE/KMessageBox>
#include <KDE/KFileDialog>
#include <KDE/KTar>
#include <KDE/KTempDir>
#include <KDE/KIO/NetAccess>
#include <KDE/KPushButton>
#include <kdeversion.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QListWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPalette>
#include <QtGui/QPainter>
#include <sys/stat.h>

enum ESelection {
    SelUsable,
    SelEnabled,
    SelDisabled,
    SelAll
};

enum Role {
    RoleUser = Qt::UserRole,
    RoleAvailable,
    RoleDir,
    RoleScript,
    RoleComment,
    RoleApp,
    RoleDBus
};

DockConfig::DockConfig(KConfigDialog *p)
{
    p->addPage(this, i18n("Dock Manager"), "preferences-system-windows");

    ui.setupUi(this);
    DockConfigItemDelegate *delegate = new DockConfigItemDelegate(ui.view, this);
    ui.view->setItemDelegate(delegate);
    ui.view->sortItems(Qt::AscendingOrder);

    QStringList dirs = DockManager::self()->dirs();
    QString home = QDir::homePath();

    foreach (QString dir, dirs) {
        QStringList metas = QDir(QString(dir + "/metadata")).entryList(QStringList() << "*.info");

        foreach (QString m, metas) {
            Entry e;
            e.user = dir.startsWith(home);
            e.script = m.left(m.length() - 5);
            e.dir = dir;

            QString script = dir + "/scripts/" + e.script;
            if (QFile::exists(script)) {
                KConfig cfg(dir + "/metadata/" + m, KConfig::NoGlobals);

                if (cfg.hasGroup("DockmanagerHelper")) {
                    KConfigGroup grp(&cfg, "DockmanagerHelper");
                    e.appName = grp.readEntry("AppName", QString());
                    e.dbusName = grp.readEntry("DBusName", QString());
                    e.description = grp.readEntry("Description", QString());
                    e.name = grp.readEntry("Name", QString());
                    if (!e.name.isEmpty() && !e.description.isEmpty()) {
                        e.icon = grp.readEntry("Icon", QString());
                        e.available = e.appName.isEmpty() || !KStandardDirs::findExe(e.appName).isEmpty();
                        e.enabled = DockManager::self()->enabledHelpers().contains(script);
                        createItem(e);
                    }
                }
            }
        }
    }

    ui.addButton->setIcon(KIcon("list-add"));
    ui.removeButton->setIcon(KIcon("list-remove"));
    ui.enable->setChecked(DockManager::self()->isEnabled());
    ui.view->setEnabled(ui.enable->isChecked());
    ui.removeButton->setEnabled(false);
    ui.addButton->setEnabled(DockManager::self()->isEnabled());

    connect(ui.view, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
    connect(ui.addButton, SIGNAL(clicked(bool)), SLOT(add()));
    connect(ui.removeButton, SIGNAL(clicked(bool)), SLOT(del()));
    connect(ui.enable, SIGNAL(toggled(bool)), SLOT(enableWidgets(bool)));
    connect(ui.enable, SIGNAL(toggled(bool)), p, SLOT(settingsModified()));
    connect(delegate, SIGNAL(changed()), this, SIGNAL(settingsModified()));
    connect(this, SIGNAL(settingsModified()), p, SLOT(settingsModified()));
}

DockConfig::~DockConfig()
{
    // Delete the item delegate, otherwise we get lots of the following printed to the screen:
    //     KWidgetItemDelegateEventListener::eventFilter: User of KWidgetItemDelegate should not delete widgets created by createItemWidgets!
    QAbstractItemDelegate *delegate = ui.view->itemDelegate();
    if (delegate) {
        delete delegate;
    }
}

bool DockConfig::isEnabled()
{
    return ui.enable->isChecked();
}

QSet<QString> DockConfig::enabledHelpers()
{
    QSet<QString> h;
    QAbstractItemModel *model = ui.view->model();

    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex idx = model->index(row, 0);

        if (model->data(idx, Qt::CheckStateRole).toBool()) {
            h.insert(model->data(idx, RoleDir).toString() + "/scripts/" + model->data(idx, RoleScript).toString());
        }
    }
    return h;
}

void DockConfig::selectionChanged()
{
    QList<QListWidgetItem*> items = ui.view->selectedItems();
    QListWidgetItem         *item = items.count() ? items.first() : 0L;

    ui.removeButton->setEnabled(ui.enable->isChecked() && item && item->data(RoleUser).toBool());
}

void DockConfig::add()
{
    KFileDialog *dlg = new KFileDialog(KUrl(), QLatin1String("application/x-bzip-compressed-tar application/x-compressed-tar application/x-tar"), this);
    dlg->setOperationMode(KFileDialog::Opening);
    dlg->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    dlg->setCaption(i18n("Open"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::WindowModal);
    connect(dlg, SIGNAL(accepted()), SLOT(fileSelected()));
    dlg->show();
}

void DockConfig::fileSelected()
{
    KFileDialog *dlg = qobject_cast<KFileDialog*>(sender());
    KUrl url = dlg ? dlg->selectedUrl() : KUrl();

    if (url.isValid()) {
        QString fileName;

        if (url.isLocalFile()) {
            fileName = url.toLocalFile();
        } else {
            if (!KIO::NetAccess::download(url, fileName, this)) {
                KMessageBox::error(this, i18n("Sorry, failed to download\n%1", url.prettyUrl()));
                return;
            }
        }

        // Uncompress...
        KTar tar(fileName);

        if (tar.open(QIODevice::ReadOnly)) {
            const KArchiveDirectory *dir = tar.directory();

            if (dir) {
                const KArchiveEntry *meta = 0,
                                     *script = 0;
                foreach (QString entry, dir->entries()) {
                    if ("scripts" == entry) {
                        const KArchiveEntry *d = dir->entry(entry);
                        if (d && d->isDirectory()) {
                            foreach (QString f, ((KArchiveDirectory *)d)->entries()) {
                                if (f.endsWith(".py")) {
                                    script = ((KArchiveDirectory *)d)->entry(f);
                                    break;
                                } else {
                                    script = ((KArchiveDirectory *)d)->entry(f);
                                }
                            }
                        } else {
                            break;
                        }
                    } else if ("metadata" == entry) {
                        const KArchiveEntry *d = dir->entry(entry);
                        if (d && d->isDirectory()) {
                            foreach (QString f, ((KArchiveDirectory *)d)->entries()) {
                                if (f.endsWith(".info")) {
                                    meta = ((KArchiveDirectory *)d)->entry(f);
                                    break;
                                }
                            }
                        } else {
                            break;
                        }
                    }

                    if (script && meta) {
                        break;
                    }
                }

                if (script && meta && meta->name() == (script->name() + ".info")) {
                    Entry e;
                    QString destDir = QString(KGlobal::dirs()->localxdgdatadir() + "/dockmanager").replace("//", "/");
                    QString error;
                    KTempDir tempDir;

                    tempDir.setAutoRemove(true);
                    e.user = true;
                    e.script = script->name();
                    e.dir = destDir;

                    // Check contents of meta data *before* attempting to install...
                    ((KArchiveFile *)meta)->copyTo(tempDir.name());
                    KConfig cfg(tempDir.name() + meta->name(), KConfig::NoGlobals);

                    if (cfg.hasGroup("DockmanagerHelper")) {
                        KConfigGroup grp(&cfg, "DockmanagerHelper");
                        e.appName = grp.readEntry("AppName", QString());
                        e.dbusName = grp.readEntry("DBusName", QString());
                        e.description = grp.readEntry("Description", QString());
                        e.name = grp.readEntry("Name", QString());
                        if (!e.name.isEmpty() && !e.description.isEmpty()) {
                            e.icon = grp.readEntry("Icon", QString());
                            e.available = e.appName.isEmpty() || !KStandardDirs::findExe(e.appName).isEmpty();
                            e.enabled = DockManager::self()->enabledHelpers().contains(destDir + "/scripts/" + script->name());
                        } else {
                            error = i18n("<p>Contents of metadata file are invalid.<ul>");
                            if (e.name.isEmpty()) {
                                error += i18n("<li><i>Name</i> field is missing.</li>");
                            }
                            if (e.description.isEmpty()) {
                                error += i18n("<li><i>Description</i> field is missing.</li>");
                            }
                            error += QLatin1String("</ul></p>");
                        }
                    } else {
                        error = i18n("<p>Metadata file does not contain <i>DockmanagerHelper</i> group.</p>");
                    }

                    if (!error.isEmpty()) {
                        KMessageBox::detailedError(this, i18n("Invalid DockManager plugin."), error);
                    } else if ((!QFile::exists(destDir + "/metadata/" + meta->name()) &&
                                !QFile::exists(destDir + "/scripts/" + script->name())) ||
                               KMessageBox::Yes == KMessageBox::warningYesNo(this, i18n("<p>A Plugin named <b>%1</b> already exists.</p>"
                                       "<p>Overwrite?</p>",
                                       script->name()),
                                       i18n("Overwrite?"))) {
                        bool abortInstall = false;

                        if (QFile::exists(destDir + "/metadata/" + meta->name()) && !QFile::remove(destDir + "/metadata/" + meta->name())) {
                            KMessageBox::error(this, i18n("<p>Sorry, failed to remove previous plugin metadata file.</p>"
                                                          "<p><i>%1</i></p>", destDir + "/metadata/" + meta->name()));
                            abortInstall = true;
                        }
                        if (!abortInstall && QFile::exists(destDir + "/scripts/" + script->name()) && !QFile::remove(destDir + "/scripts/" + script->name())) {
                            KMessageBox::error(this, i18n("<p>Sorry, failed to remove previous plugin metadata file.</p>"
                                                          "<p><i>%1</i></p>", destDir + "/scripts/" + script->name()));
                            abortInstall = true;
                        }
                        if (!abortInstall && (!(QDir(destDir + "/scripts/").exists() || KStandardDirs::makeDir(destDir + "/scripts/")))) {
                            KMessageBox::error(this, i18n("<p>Sorry, failed to create scripts folder.</p>"
                                                          "<p><i>%1</i></p>", destDir + "/scripts/"));
                            abortInstall = true;
                        }
                        if (!abortInstall && (!(QDir(destDir + "/metadata/").exists() || KStandardDirs::makeDir(destDir + "/metadata/")))) {
                            KMessageBox::error(this, i18n("<p>Sorry, failed to create metadata folder.</p>"
                                                          "<p><i>%1</i></p>", destDir + "/metadata/"));
                            abortInstall = true;
                        }
                        if (!abortInstall) {
                            ((KArchiveFile *)script)->copyTo(destDir + "/scripts/");
                            if (!QFile::exists(destDir + "/scripts/" + script->name())) {
                                KMessageBox::error(this, i18n("Sorry, failed to install script file."));
                                abortInstall = true;
                            }
                        }
                        if (!abortInstall) {
                            ((KArchiveFile *)meta)->copyTo(destDir + "/metadata/");
                            if (!QFile::exists(destDir + "/metadata/" + meta->name())) {
                                KMessageBox::error(this, i18n("Sorry, failed to install metadata file."));
                                abortInstall = true;
                            }
                        }

                        if (!abortInstall) {
                            // Make sure script is executable...
                            // Clear any umask before setting file perms
                            mode_t oldMask(umask(0000));
                            ::chmod(QFile::encodeName(destDir + "/scripts/" + script->name()).constData(), 0755);
                            // Reset umask
                            ::umask(oldMask);

                            QListWidgetItem *item = createItem(e);
                            foreach (QListWidgetItem * i, ui.view->selectedItems()) {
                                i->setSelected(false);
                            }
                            item->setSelected(true);
                            ui.view->scrollToItem(item);
                        }
                    }
                } else {
                    QString error = QLatin1String("<p><ul>");
                    if (!script) {
                        error += i18n("<li>Script file is missing.</li>");
                    }
                    if (!meta) {
                        error += i18n("<li>Metadata file is missing.</li>");
                    }
                    error += QLatin1String("</ul></p>");
                    KMessageBox::detailedError(this, i18n("Invalid DockManager plugin."), error);
                }
            }
        }

        if (!url.isLocalFile()) {
            KIO::NetAccess::removeTempFile(fileName);
        }
    }
}

void DockConfig::del()
{
    QList<QListWidgetItem*> items = ui.view->selectedItems();
    QListWidgetItem         *item = items.count() ? items.first() : 0L;

    if (item && item->data(RoleUser).toBool() &&
            KMessageBox::Yes == KMessageBox::warningYesNo(this, i18n("<p>Are you sure you wish to delete <b>%1</b></p><p><i>(%2)</i></p>",
                    item->text(), item->data(RoleScript).toString()),
                    i18n("Remove Script"))) {
        QString dir = item->data(RoleDir).toString(),
                script = item->data(RoleScript).toString();
        if (QFile::remove(dir + "/scripts/" + script) && QFile::remove(dir + "/metadata/" + script + ".info")) {
            int row = ui.view->row(item);
            QListWidgetItem *other = ui.view->item(row + 1);

            if (!other && row > 0) {
                other = ui.view->item(row - 1);
            }
            delete item;
            if (other) {
                other->setSelected(true);
            }
            emit settingsModified();
        } else {
            KMessageBox::error(this, i18n("<p>Failed to delete the script file.</p><p><i>%1</i></p>", dir + "/scripts/" + script));
        }
    }
}

void DockConfig::enableWidgets(bool e)
{
    if (e) {
        QList<QListWidgetItem*> items = ui.view->selectedItems();
        QListWidgetItem         *item = items.count() ? items.first() : 0L;

        ui.removeButton->setEnabled(item && item->data(RoleUser).toBool());
    } else {
        ui.removeButton->setEnabled(false);
    }
    ui.addButton->setEnabled(e);
    ui.view->setEnabled(e);
}

QListWidgetItem * DockConfig::createItem(const Entry &e)
{
    QListWidgetItem *item = new QListWidgetItem(ui.view);
    item->setText(e.name);
    item->setData(RoleComment, e.description);
    item->setData(Qt::DecorationRole, e.icon);
    item->setCheckState(e.available && e.enabled ? Qt::Checked : Qt::Unchecked);
    item->setData(RoleUser, e.user);
    item->setData(RoleAvailable, e.available);
    item->setData(RoleDir, e.dir);
    item->setData(RoleScript, e.script);
    item->setData(RoleApp, e.appName);
    item->setData(RoleDBus, e.dbusName);
    return item;
}

static const int constMargin = 5;

DockConfigItemDelegate::DockConfigItemDelegate(QAbstractItemView *itemView, QObject *parent)
    : KWidgetItemDelegate(itemView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new KPushButton)
{
    pushButton->setIcon(KIcon("configure")); // only for getting size matters
}

DockConfigItemDelegate::~DockConfigItemDelegate()
{
    delete checkBox;
    delete pushButton;
}

int DockConfigItemDelegate::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (itemView()->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

void DockConfigItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    int xOffset = checkBox->sizeHint().width();
    bool disabled = !itemView()->isEnabled() || !index.model()->data(index, RoleAvailable).toBool();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    int iconSize = option.rect.height() - constMargin * 2;
    QPixmap pixmap = KIconLoader::global()->loadIcon(index.model()->data(index, Qt::DecorationRole).toString(),
                     KIconLoader::Desktop, iconSize, disabled ? KIconLoader::DisabledState : KIconLoader::DefaultState);
    painter->drawPixmap(QRect(dependantLayoutValue(constMargin + option.rect.left() + xOffset, iconSize, option.rect.width()), constMargin + option.rect.top(), iconSize, iconSize),
                        pixmap, QRect(0, 0, iconSize, iconSize));


    QRect contentsRect(dependantLayoutValue(constMargin * 2 + iconSize + option.rect.left() + xOffset, option.rect.width() - constMargin * 3 - iconSize - xOffset, option.rect.width()),
                       constMargin + option.rect.top(), option.rect.width() - constMargin * 3 - iconSize - xOffset, option.rect.height() - constMargin * 2);
    int lessHorizontalSpace = constMargin * 2 + pushButton->sizeHint().width();

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    if (itemView()->layoutDirection() == Qt::RightToLeft) {
        contentsRect.translate(lessHorizontalSpace, 0);
    }

    painter->save();
    if (disabled) {
        QPalette pal(option.palette);
        pal.setCurrentColorGroup(QPalette::Disabled);
        painter->setPen(pal.text().color());
    }

    painter->save();
    QFont font = titleFont(option.font);
    bool system = !index.model()->data(index, RoleUser).toBool();

    font.setItalic(system);

    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    font = painter->font();
    font.setItalic(system);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, RoleComment).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();
    painter->restore();
}

QSize DockConfigItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()) + KIconLoader::SizeMedium + constMargin * 5 + pushButton->sizeHint().width(),
                 qMax(KIconLoader::SizeMedium + constMargin * 2, fmTitle.height() + option.fontMetrics.height() + constMargin * 2));
}

QList<QWidget*> DockConfigItemDelegate::createItemWidgets() const
{
    QList<QWidget*> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(itemToggled(bool)));

    KPushButton *aboutPushButton = new KPushButton;
    aboutPushButton->setIcon(KIcon("dialog-information"));
    connect(aboutPushButton, SIGNAL(clicked(bool)), this, SLOT(aboutClicked()));

    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                         << QEvent::KeyPress << QEvent::KeyRelease);

    setBlockedEventTypes(aboutPushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                         << QEvent::KeyPress << QEvent::KeyRelease);

    widgetList << enabledCheckBox << aboutPushButton;

    return widgetList;
}

void DockConfigItemDelegate::updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(dependantLayoutValue(constMargin, checkBox->sizeHint().width(), option.rect.width()), option.rect.height() / 2 - checkBox->sizeHint().height() / 2);

    KPushButton *aboutPushButton = static_cast<KPushButton*>(widgets[1]);
    QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(dependantLayoutValue(option.rect.width() - constMargin - aboutPushButtonSizeHint.width(), aboutPushButtonSizeHint.width(), option.rect.width()),
                          option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        aboutPushButton->setVisible(false);
    } else {
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        checkBox->setEnabled(index.model()->data(index, RoleAvailable).toBool());
    }
}

QFont DockConfigItemDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

void DockConfigItemDelegate::itemToggled(bool e)
{
    const QModelIndex index = focusedIndex();

    if (!index.isValid()) {
        return;
    }

    const_cast<QAbstractItemModel*>(focusedIndex().model())->setData(index, e, Qt::CheckStateRole);
    emit changed();
}

void DockConfigItemDelegate::aboutClicked()
{
    const QModelIndex index = focusedIndex();

    if (!index.isValid()) {
        return;
    }

    const QAbstractItemModel *model = index.model();
    QString appName(model->data(index, RoleApp).toString());
    QString dbusName(model->data(index, RoleDBus).toString());

    KMessageBox::information(itemView(),
                             QString("%1<hr style=\"height: 1px;border: 0px\" />").arg(model->data(index, RoleComment).toString()) +
                             QString("<table>") +
                             i18n("<tr><td align=\"right\">Script File:</td><td>%1</td></tr>", model->data(index, RoleScript).toString()) +
                             i18n("<tr><td align=\"right\">Location:</td><td>%1</td></tr>", model->data(index, RoleDir).toString()) +
                             (appName.isEmpty() ? QString() : i18n("<tr><td align=\"right\">Application:</td><td>%1</td></tr>", appName)) +
                             (dbusName.isEmpty() ? QString() : i18n("<tr><td align=\"right\">D-Bus:</td><td>%1</td></tr>", dbusName)) +
                             QString("</table>"),
                             model->data(index, Qt::DisplayRole).toString()
#if KDE_IS_VERSION(4, 7, 0)
                             ,QString(), KMessageBox::WindowModal
#endif
                             );
}

#include "dockconfig.moc"
