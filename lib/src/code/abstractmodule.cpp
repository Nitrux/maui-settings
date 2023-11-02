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

bool AbstractModule::checkFilter(const QRegularExpression &filter)
{
    bool keywordContains = false;

    for(const auto &keyword : m_keywords)
    {
        if(keyword.contains(filter))
        {
            keywordContains = true;
            break;
        }
    }

    return m_name.contains(filter) || m_category.contains(filter) || m_description.contains(filter) || m_id.contains(filter) || keywordContains;
}
