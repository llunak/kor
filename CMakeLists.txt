cmake_minimum_required( VERSION 2.8.12 FATAL_ERROR )

project( kor )

option( KOR_DESKTOPS "Build Kor support for desktop background" OFF )
option( KOR_MINICLI "Build Kor support for mini command line" ON )
option( KOR_PANELS "Build Kor support for panels" OFF )

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMPackageConfigHelpers)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

set(KF5_MIN_VERSION "5.24.0")
find_package(KF5 REQUIRED COMPONENTS
    Config
    CoreAddons
    GlobalAccel
    I18n
    Init
    Service
    KDELibs4Support
)

# Set KI18n translation domain
add_definitions(-DTRANSLATION_DOMAIN=\"kor\")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

########### next target ###############

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_BINARY_DIR}
    )

if(KOR_DESKTOPS)
include_directories( ${CMAKE_SOURCE_DIR}/desktop )
endif(KOR_DESKTOPS)

if(KOR_MINICLI)
include_directories( ${CMAKE_SOURCE_DIR}/minicli )
endif(KOR_MINICLI)

if(KOR_PANELS)
include_directories( ${CMAKE_SOURCE_DIR}/panels )
endif(KOR_PANELS)

########### next target ###############

set( kor_SRCS
     main.cpp
     application.cpp
#     startupsuspendhandler.cpp
     )

if(KOR_DESKTOPS)
set( kor_SRCS ${kor_SRCS}
     desktop/desktop.cpp
     desktop/plasmawallpaper.cpp
     desktop/wallpaper.cpp
     utils/x11utils.cpp
     )
endif(KOR_DESKTOPS)

if(KOR_MINICLI)
set( kor_SRCS ${kor_SRCS}
     minicli/minicli.cpp
     minicli/miniclidialog.cpp
     minicli/miniclihandler.cpp
     )
ki18n_wrap_ui( kor_SRCS
    minicli/miniclidialog.ui
    )
set( kor_KCFGC_FILES ${kor_KCFGC_FILES}
     minicli/minicliconfig.kcfgc
     minicli/miniclidialogconfig.kcfgc
     )
endif(KOR_MINICLI)

if(KOR_PANELS)
set( kor_SRCS ${kor_SRCS}
     panels/applet.cpp
     panels/panel.cpp
     panels/spacer.cpp
     panels/clock/clock.cpp
     panels/hello/hello.cpp
     panels/plasma/plasmaapplet.cpp
     )
if(HAVE_NETROOTINFO2_CURRENTDESKTOPFORSCREEN)
set( kor_SRCS ${kor_SRCS}
     panels/desktopsonscreens/desktopsonscreens.cpp
     )
endif(HAVE_NETROOTINFO2_CURRENTDESKTOPFORSCREEN)
set( kor_KCFGC_FILES ${kor_KCFGC_FILES}
     panels/clock/clockconfig.kcfgc
     panels/hello/helloconfig.kcfgc
     panels/plasma/plasmaappletconfig.kcfgc
     )
endif(KOR_PANELS)

kconfig_add_kcfg_files( kor_SRCS ${kor_KCFGC_FILES} )
foreach( kcfgc ${kor_KCFGC_FILES} )
    get_filename_component( kcfgc_path ${kcfgc} PATH )
    get_filename_component( kcfgc_name ${kcfgc} NAME_WE )
    set( kor_KCFG_FILES ${kor_KCFG_FILES} ${kcfgc_path}/${kcfgc_name}.kcfg )
endforeach( kcfgc ${kor_KCFGC_FILES} )

kf5_add_kdeinit_executable( kor ${kor_SRCS} )
target_link_libraries( kdeinit_kor
    Qt5::Widgets
    KF5::ConfigCore
    KF5::GlobalAccel
    KF5::Service
    KF5::KDELibs4Support
    )

install( TARGETS kor ${INSTALL_TARGETS_DEFAULT_ARGS})
install( TARGETS kdeinit_kor ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES kor.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install( FILES ${kor_KCFG_FILES} DESTINATION ${KCFG_INSTALL_DIR}/kor )
install( FILES korrc DESTINATION ${CONFIG_INSTALL_DIR} )

########### next target ###############

if(KOR_PANELS)
kde4_add_plugin(plasma_containment_korinternal panels/plasma/plasmacontainment.cpp)
if( KDE4_PLASMA_LIBS )
    target_link_libraries(plasma_containment_korinternal ${KDE4_PLASMA_LIBS})
else( KDE4_PLASMA_LIBS )
    target_link_libraries(plasma_containment_korinternal plasma ${KDE4_KIO_LIBS})
endif( KDE4_PLASMA_LIBS )

install(TARGETS plasma_containment_korinternal DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES panels/plasma/plasma-containment-korinternal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
endif(KOR_PANELS)
