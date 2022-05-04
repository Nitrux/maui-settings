#ifndef MODULEITEM_H
#define MODULEITEM_H

#include <QObject>

class ModuleItem : public QObject
{
    Q_OBJECT
public:
    explicit ModuleItem(QObject *parent = nullptr);

signals:

};

#endif // MODULEITEM_H
