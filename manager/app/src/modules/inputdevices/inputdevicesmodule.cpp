#include "inputdevicesmodule.h"

#include <MauiMan/inputdevicesmanager.h>
#include <KI18n/KLocalizedString>

InputDevicesModule::InputDevicesModule(QObject *parent) : AbstractModule(QStringLiteral("inputdevices"),
                                                                         i18n("Input Devices"),
                                                                         i18n("System"),
                                                                         QStringLiteral("qrc:/modules/inputdevices/InputDevicesModuleView.qml"),
                                                                         QStringLiteral("preferences-desktop-peripherals"),
                                                                         i18n("Configure your input devices."),
                                                                         QStringList{"mouse", "keyboard", "keymap", "variant", "layout"},
                                                                         parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<MauiMan::InputDevicesManager*>("const MauiMan::InputDevicesManager*"); // this is needed for QML to know of FormFactorManager

}

MauiMan::InputDevicesManager *InputDevicesModule::manager()
{
    if(!m_manager)
    {
        m_manager = new MauiMan::InputDevicesManager(this);
    }

    return m_manager;
}
