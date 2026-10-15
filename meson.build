project('maui-settings', 'cpp', version: '0.1.0', meson_version: '>=1.12.0', default_options: ['cpp_std=c++23', 'buildtype=release'])
cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc' and cpp_compiler.has_argument('-Wno-sfinae-incomplete')
    add_project_arguments('-Wno-sfinae-incomplete', language: 'cpp')
endif
qt6 = import('qt6')
kf6_include_prefix = get_option('prefix') / 'include' / 'KF6'
qt = dependency('qt6', version: '>=6.9', modules: ['Core', 'DBus', 'Gui', 'Network', 'Qml', 'Quick', 'Widgets'])
kf6_auth = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KAuthCore', kf6_include_prefix / 'KAuth'),
    dependencies: [cpp_compiler.find_library('KF6AuthCore')],
)
kf6_bluez = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'BluezQt'),
    dependencies: [cpp_compiler.find_library('KF6BluezQt')],
)
kf6_config = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KConfig', kf6_include_prefix / 'KConfigCore', kf6_include_prefix / 'KConfigGui'),
    dependencies: [cpp_compiler.find_library('KF6ConfigCore'), cpp_compiler.find_library('KF6ConfigGui')],
)
kf6_colorscheme = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KColorScheme'),
    dependencies: [cpp_compiler.find_library('KF6ColorScheme')],
)
kf6_core = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KCoreAddons'),
    dependencies: [cpp_compiler.find_library('KF6CoreAddons')],
)
kf6_i18n = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KI18n'),
    dependencies: [cpp_compiler.find_library('KF6I18n'), cpp_compiler.find_library('KF6I18nQml')],
)
kf6_networkmanager = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'NetworkManagerQt'),
    dependencies: [cpp_compiler.find_library('KF6NetworkManagerQt')],
)
kf6_modem = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'ModemManagerQt', get_option('prefix') / 'include' / 'ModemManager'),
    dependencies: [cpp_compiler.find_library('KF6ModemManagerQt')],
)
mauikit = dependency('MauiKit4', version: '>=4.0.5')
mauikit_filebrowsing = dependency('MauiKitFileBrowsing4', version: '>=4.0.5')
system_control = declare_dependency(
    include_directories: include_directories(get_option('prefix') / 'include' / 'Maui' / 'Core' / 'System'),
    dependencies: [cpp_compiler.find_library('MauiKitSystemControl')],
)
system_network = declare_dependency(
    include_directories: include_directories(get_option('prefix') / 'include' / 'Maui' / 'Core' / 'Network'),
    dependencies: [cpp_compiler.find_library('MauiKitSystemNetwork')],
)
mauiman = declare_dependency(
    include_directories: include_directories(get_option('prefix') / 'include'),
    dependencies: [cpp_compiler.find_library('MauiMan4')],
)
qcoro_include_prefix = get_option('prefix') / 'include' / 'qcoro6'
qcoro_core = declare_dependency(
    include_directories: include_directories(qcoro_include_prefix, qcoro_include_prefix / 'QCoro', qcoro_include_prefix / 'qcoro'),
    dependencies: [cpp_compiler.find_library('QCoro6Core')],
)
qcoro_dbus = declare_dependency(
    include_directories: include_directories(qcoro_include_prefix, qcoro_include_prefix / 'QCoro', qcoro_include_prefix / 'qcoro'),
    dependencies: [cpp_compiler.find_library('QCoro6DBus')],
)
networkmanager = dependency('libnm', required: true)
x11cursor = dependency('xcursor', required: true)
pwquality = dependency('pwquality', required: true)

git_program = find_program('git', required: false)
git_branch = 'Stable'
git_commit_hash = '0.0.1'
if git_program.found()
    git_branch_result = run_command(git_program, '-C', meson.project_source_root(), 'rev-parse', '--abbrev-ref', 'HEAD', check: false)
    git_commit_result = run_command(git_program, '-C', meson.project_source_root(), 'rev-parse', '--short', 'HEAD', check: false)
    if git_branch_result.returncode() == 0 and git_branch_result.stdout().strip() != ''
        git_branch = git_branch_result.stdout().strip()
    endif
    if git_commit_result.returncode() == 0 and git_commit_result.stdout().strip() != ''
        git_commit_hash = git_commit_result.stdout().strip()
    endif
endif

cpp_args = ['-DGIT_BRANCH="' + git_branch + '"', '-DGIT_COMMIT_HASH="' + git_commit_hash + '"']
if host_machine.cpu_family() == 'x86_64'
    cpp_args += ['-march=x86-64-v3']
endif
settings_includes = include_directories('src')

settings_sources = [
    'src/main.cpp',
    'src/controllers/aboutinfo.cpp', 'src/controllers/audiocontroller.cpp', 'src/controllers/systemmanager.cpp',
    'src/controllers/backgroundinfo.cpp', 'src/controllers/wallpapercontroller.cpp', 'src/controllers/wallpapercolorscontroller.cpp',
    'src/controllers/bluetoothcontroller.cpp', 'src/controllers/desklockcontroller.cpp', 'src/controllers/kdeglobalsinfo.cpp',
    'src/controllers/gtksettingsinfo.cpp', 'src/controllers/qmlgreetcontroller.cpp', 'src/controllers/qmllogoutcontroller.cpp',
    'src/controllers/marinainfo.cpp', 'src/controllers/mimecontroller.cpp', 'src/controllers/nudgeosdinfo.cpp',
    'src/controllers/networkcontroller.cpp', 'src/controllers/valenzinfo.cpp', 'src/controllers/tomainfo.cpp',
    'src/controllers/powerinfo.cpp', 'src/controllers/graphicsmodecontroller.cpp', 'src/controllers/batterycontroller.cpp',
    'src/controllers/hyprsunsetinfo.cpp', 'src/controllers/hyprlandinfo.cpp', 'src/controllers/hyprlandkeybinds.cpp',
    'src/controllers/hyprlanddevices.cpp', 'src/controllers/hyprlandrules.cpp', 'src/controllers/autostartcontroller.cpp',
    'src/controllers/environmentcontroller.cpp', 'src/controllers/flatpakpermissionscontroller.cpp', 'src/controllers/displayscontroller.cpp',
]
settings_moc = qt6.preprocess(
    qresources: 'src/qml.qrc',
    dependencies: [qt, kf6_networkmanager, kf6_modem, networkmanager, qcoro_core, qcoro_dbus, system_network],
    moc_extra_arguments: ['-I' + qt.get_variable('includedir'), '-I' + qt.get_variable('includedir') + '/QtQml'],
    moc_sources: ['src/controllers/networkcontroller.cpp'],
    moc_headers: [
        'src/controllers/networkcontroller.h', 'src/controllers/aboutinfo.h', 'src/controllers/audiocontroller.h', 'src/controllers/systemmanager.h',
        'src/controllers/backgroundinfo.h', 'src/controllers/wallpapercontroller.h', 'src/controllers/wallpapercolorscontroller.h',
        'src/controllers/bluetoothcontroller.h', 'src/controllers/desklockcontroller.h', 'src/controllers/kdeglobalsinfo.h',
        'src/controllers/gtksettingsinfo.h', 'src/controllers/qmlgreetcontroller.h', 'src/controllers/qmllogoutcontroller.h',
        'src/controllers/marinainfo.h', 'src/controllers/mimecontroller.h', 'src/controllers/nudgeosdinfo.h',
        'src/controllers/valenzinfo.h', 'src/controllers/tomainfo.h',
        'src/controllers/powerinfo.h', 'src/controllers/graphicsmodecontroller.h', 'src/controllers/batterycontroller.h',
        'src/controllers/hyprsunsetinfo.h', 'src/controllers/hyprlandinfo.h',
        'src/controllers/autostartcontroller.h',
        'src/controllers/environmentcontroller.h', 'src/controllers/flatpakpermissionscontroller.h', 'src/controllers/displayscontroller.h',
    ],
)

desktop = configure_file(input: 'org.maui.settings.desktop.in', output: 'org.maui.settings.desktop', configuration: {'APP_NAME': 'maui-settings'})
executable('maui-settings', settings_sources + settings_moc, include_directories: settings_includes, dependencies: [qt, kf6_auth, kf6_bluez, kf6_config, kf6_colorscheme, kf6_core, kf6_i18n, kf6_networkmanager, kf6_modem, networkmanager, qcoro_core, qcoro_dbus, mauikit, mauikit_filebrowsing, system_control, system_network, mauiman, x11cursor], cpp_args: cpp_args, install: true)

kauth_dir = get_option('libdir') / 'libexec' / 'kf6' / 'kauth'
qmlgreet_helper_moc = qt6.preprocess(moc_headers: ['src/helpers/qmlgreethelper.h'], dependencies: [qt, kf6_auth])
datetime_helper_moc = qt6.preprocess(moc_headers: ['src/helpers/datetimehelper.h'], dependencies: [qt, kf6_auth])
graphics_helper_moc = qt6.preprocess(moc_headers: ['src/helpers/graphicsmodehelper.h'], dependencies: [qt, kf6_auth])
qmlgreet_helper = executable('maui-settings-qmlgreet-helper', ['src/helpers/qmlgreethelper.cpp', 'src/helpers/systemfilepersistence.cpp'] + qmlgreet_helper_moc, include_directories: settings_includes, dependencies: [qt, kf6_auth], install: true, install_dir: kauth_dir)
datetime_helper = executable('maui-settings-datetime-helper', ['src/helpers/datetimehelper.cpp', 'src/helpers/systemfilepersistence.cpp'] + datetime_helper_moc, include_directories: settings_includes, dependencies: [qt, kf6_auth, pwquality], cpp_args: ['-DMAUI_SETTINGS_HAS_PWQUALITY'], install: true, install_dir: kauth_dir)
graphics_helper = executable('maui-settings-graphics-helper', ['src/helpers/graphicsmodehelper.cpp'] + graphics_helper_moc, include_directories: settings_includes, dependencies: [qt, kf6_auth], install: true, install_dir: kauth_dir)
gtk3 = dependency('gtk+-3.0', method: 'pkg-config', required: false)
gtk4 = dependency('gtk4', method: 'pkg-config', required: false)
if gtk3.found()
    executable('maui-settings-gtk3-preview-helper', 'src/helpers/gtkpreviewhelper.cpp', include_directories: settings_includes, dependencies: [gtk3], cpp_args: ['-DMAUI_SETTINGS_GTK3'], install: true)
endif
if gtk4.found()
    executable('maui-settings-gtk4-preview-helper', 'src/helpers/gtkpreviewhelper.cpp', include_directories: settings_includes, dependencies: [gtk4], cpp_args: ['-DMAUI_SETTINGS_GTK4'], install: true)
endif

qmlgreet_conf = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.qmlgreet.conf',
    configuration: {'CONTENT': ('''<!DOCTYPE busconfig PUBLIC
 "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="root">
    <allow own="org.maui.settings.qmlgreet"/>
  </policy>
  <policy context="default">
    <allow send_destination="org.maui.settings.qmlgreet"/>
  </policy>
</busconfig>
''')}
)
qmlgreet_service = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.qmlgreet.service',
    configuration: {'CONTENT': (('''[D-BUS Service]
Name=org.maui.settings.qmlgreet
Exec=@HELPER_DIR@/maui-settings-qmlgreet-helper
User=root
''')).replace('@HELPER_DIR@', (kauth_dir))}
)
qmlgreet_policy = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.qmlgreet.policy',
    configuration: {'CONTENT': ('''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE policyconfig PUBLIC
"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
<policyconfig>
<vendor>QMLGreet settings</vendor>
   <action id="org.maui.settings.qmlgreet.copykdeglobals">
      <description>Copy KDE globals to QMLGreet</description>
      <message>Authentication is required to update the greetd user KDE and cursor settings</message>
      <defaults>
         <allow_inactive>no</allow_inactive>
         <allow_active>auth_admin</allow_active>
      </defaults>
   </action>
   <action id="org.maui.settings.qmlgreet.save">
      <description>Save QMLGreet settings</description>
      <message>Authentication is required to modify the system-wide QMLGreet configuration</message>
      <defaults>
         <allow_inactive>no</allow_inactive>
         <allow_active>auth_admin</allow_active>
      </defaults>
   </action>
</policyconfig>
''')}
)

datetime_conf = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.datetime.conf',
    configuration: {'CONTENT': ('''<!DOCTYPE busconfig PUBLIC
 "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="root">
    <allow own="org.maui.settings.datetime"/>
  </policy>
  <policy context="default">
    <allow send_destination="org.maui.settings.datetime"/>
  </policy>
</busconfig>
''')}
)
datetime_service = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.datetime.service',
    configuration: {'CONTENT': (('''[D-BUS Service]
Name=org.maui.settings.datetime
Exec=@HELPER_DIR@/maui-settings-datetime-helper
User=root
''')).replace('@HELPER_DIR@', (kauth_dir))}
)
datetime_policy = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.datetime.policy',
    configuration: {'CONTENT': ('''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE policyconfig PUBLIC
"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
<policyconfig>
<vendor>System settings</vendor>
   <action id="org.maui.settings.datetime.set">
      <description>Change System settings</description>
      <message>Authentication is required to modify system hostname, locale, timezone, clock, and user accounts</message>
      <defaults>
         <allow_inactive>no</allow_inactive>
         <allow_active>auth_admin</allow_active>
      </defaults>
   </action>
</policyconfig>
''')}
)

graphics_conf = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.graphics.conf',
    configuration: {'CONTENT': ('''<!DOCTYPE busconfig PUBLIC
 "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="root">
    <allow own="org.maui.settings.graphics"/>
  </policy>
  <policy context="default">
    <allow send_destination="org.maui.settings.graphics"/>
  </policy>
</busconfig>
''')}
)
graphics_service = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.graphics.service',
    configuration: {'CONTENT': (('''[D-BUS Service]
Name=org.maui.settings.graphics
Exec=@HELPER_DIR@/maui-settings-graphics-helper
User=root
''')).replace('@HELPER_DIR@', (kauth_dir))}
)
graphics_policy = configure_file(
    input: 'meson-content.in',
    output: 'org.maui.settings.graphics.policy',
    configuration: {'CONTENT': ('''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE policyconfig PUBLIC
"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
<policyconfig>
<vendor>Graphics settings</vendor>
   <action id="org.maui.settings.graphics.set">
      <description>Change graphics mode</description>
      <message>Authentication is required to change the system graphics mode</message>
      <defaults>
         <allow_inactive>no</allow_inactive>
         <allow_active>auth_admin</allow_active>
      </defaults>
   </action>
</policyconfig>
''')}
)

install_data([
    qmlgreet_conf,
    datetime_conf,
    graphics_conf,
], install_dir: get_option('datadir') / 'dbus-1' / 'system.d')
install_data([
    qmlgreet_service,
    datetime_service,
    graphics_service,
], install_dir: get_option('datadir') / 'dbus-1' / 'system-services')
install_data([
    qmlgreet_policy,
    datetime_policy,
    graphics_policy,
], install_dir: get_option('datadir') / 'polkit-1' / 'actions')
install_data(desktop, install_dir: get_option('datadir') / 'applications')
