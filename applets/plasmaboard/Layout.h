#ifndef LAYOUT_H
#define LAYOUT_H

#include <QString>



class Layout
{
public:
    Layout(QString path);
    QString name();
    QString path();

private:
    QString m_name;
    QString m_path;
};

#endif // LAYOUT_H
