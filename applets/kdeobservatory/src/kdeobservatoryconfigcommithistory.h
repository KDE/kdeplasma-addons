#ifndef KDEOBSERVATORYCONFIGCOMMITHISTORY_HEADER
#define KDEOBSERVATORYCONFIGCOMMITHISTORY_HEADER

#include <QWidget>
#include "ui_kdeobservatoryconfigcommithistory.h"

class KdeObservatoryConfigCommitHistory : public QWidget, public Ui::KdeObservatoryConfigCommitHistory
{
    Q_OBJECT
public:
    KdeObservatoryConfigCommitHistory(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KdeObservatoryConfigCommitHistory();

    void createListWidgetItem(const QString &projectName, const QString &icon, bool active);

public Q_SLOTS:
    void projectAdded(const QString &projectName, const QString &icon);
    void projectRemoved(const QString &projectName);
};

#endif
