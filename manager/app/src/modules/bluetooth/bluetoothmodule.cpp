#include "bluetoothmodule.h"
//#include "theme.h"


BluetoothModule::BluetoothModule(QObject *parent) :AbstractModule("bluetooth", parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


QString BluetoothModule::qmlSource() const
{
    return ("qrc:/modules/theme/BluetoothModuleView.qml");
}

QString BluetoothModule::iconName() const
{
    return "preferences-system-bluetooth";
}

QString BluetoothModule::name() const
{
    return "Bluetooth";
}

QString BluetoothModule::category() const
{
    return "Network";
}

QStringList BluetoothModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString BluetoothModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

//Theme *BluetoothModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
