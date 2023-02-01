#include "abstractmodule.h"

AbstractModule::AbstractModule(const QString &id, QObject *parent) : QObject(parent)
  ,m_id(id)
{

}
