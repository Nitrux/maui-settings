#include "audiomodule.h"
//#include "theme.h"
#include <KI18n/KLocalizedString>


AudioModule::AudioModule(QObject *parent) :AbstractModule("audio",
                                                          i18n("Audio"),
                                                          i18n("System"),
                                                          "qrc:/modules/theme/AudioModuleView.qml",
                                                          "preferences-desktop-sound",
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                          {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                          parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}

