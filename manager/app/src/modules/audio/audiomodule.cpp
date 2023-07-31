#include "audiomodule.h"
//#include "theme.h"
#include <KLocalizedString>


AudioModule::AudioModule(QObject *parent) :AbstractModule(QStringLiteral("audio"),
                                                          i18n("Audio"),
                                                          i18n("System"),
                                                          QStringLiteral("qrc:/MauiSettings/src/modules/theme/AudioModuleView.qml"),
                                                          QStringLiteral("preferences-desktop-sound"),
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),

                                                          QStringList{i18n("audio"),
                                                          i18n("sound"),
                                                          i18n("mixer"),
                                                          i18n("headphones"),
                                                          i18n("speaker"),
                                                          i18n("mic"),
                                                          i18n("microphone")},

                                                          parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}

