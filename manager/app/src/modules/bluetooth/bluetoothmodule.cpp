#include "bluetoothmodule.h"
//#include "theme.h"
#include <KI18n/KLocalizedString>


BluetoothModule::BluetoothModule(QObject *parent) :AbstractModule(QStringLiteral("bluetooth"),
                                                                  i18n("Bluetooth"),
                                                                  i18n("Network"),
                                                                  QStringLiteral("qrc:/modules/theme/BluetoothModuleView.qml"),
                                                                  QStringLiteral("love"),
                                                                  i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                                  QStringList{"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                                  parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}

//Theme *BluetoothModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
