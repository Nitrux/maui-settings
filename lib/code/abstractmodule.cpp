#include "abstractmodule.h"

AbstractModule::AbstractModule(const QString &id, const QString &name, const QString &category, const QString &qmlSource, const QString &iconName, const QString &description, const QStringList &keywords, QObject *parent) : QObject(parent)
  ,m_id(id)
  ,m_name(name)
  ,m_category(category)
  ,m_qmlSource(qmlSource)
  ,m_iconName(iconName)
  ,m_description(description)
  ,m_keywords(keywords)
{

}
