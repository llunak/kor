cmake_minimum_required( VERSION 2.6 )

project( kor )

option( KOR_DESKTOPS "Build Kor support for desktop background" ON )
option( KOR_MINICLI "Build Kor support for mini command line" ON )
option( KOR_PANELS "Build Kor support for panels" ON )

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

find_package( KDE4 REQUIRED )
# older KDE versions do not have this cmake module
# find_package( KDE4Workspace REQUIRED )
find_path( KDE4WORKSPACE_INCLUDE_DIR NAMES kworkspace/kworkspace.h PATHS ${KDE4_INCLUDES} )
find_library( KDE4WORKSPACE_KWORKSPACE_LIBS NAMES kworkspace PATHS ${KDE4_LIB_DIR} )
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( KDE4Workspace DEFAULT_MSG KDE4WORKSPACE_KWORKSPACE_LIBS KDE4WORKSPACE_INCLUDE_DIR )
if( NOT KDE4WORKSPACE_FOUND )
    message( FATAL_ERROR "KDE Workspace development files not found." )
endif( NOT KDE4WORKSPACE_FOUND )

include( MacroLibrary )
include( KDE4Defaults )
add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1 )
include_directories( ${KDE4_INCLUDES} ${KDE4WORKSPACE_INCLUDE_DIR} )

include(CheckNetRootInfo2CurrentDesktopForScreen)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

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
     startupsuspendhandler.cpp
     )

if(KOR_DESKTOPS)
set( kor_SRCS ${kor_SRCS}
     desktop/desktop.cpp
     desktop/plasmawallpaper.cpp
     desktop/wallpaper.cpp
     utils/x11utils.cpp
     )
endif(KOR_PANELS)

if(KOR_MINICLI)
set( kor_SRCS ${kor_SRCS}
     minicli/minicli.cpp
     minicli/miniclidialog.cpp
     minicli/miniclihandler.cpp
     )
kde4_add_ui_files( kor_SRCS
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

kde4_add_kcfg_files( kor_SRCS ${kor_KCFGC_FILES} )
foreach( kcfgc ${kor_KCFGC_FILES} )
    get_filename_component( kcfgc_path ${kcfgc} PATH )
    get_filename_component( kcfgc_name ${kcfgc} NAME_WE )
    set( kor_KCFG_FILES ${kor_KCFG_FILES} ${kcfgc_path}/${kcfgc_name}.kcfg )
endforeach( kcfgc ${kor_KCFGC_FILES} )

kde4_add_kdeinit_executable( kor ${kor_SRCS} )
target_link_libraries( kdeinit_kor
    ${KDE4_KIO_LIBS}
    ${KDE4WORKSPACE_KWORKSPACE_LIBS}
    ${X11_LIBRARIES})
if( KDE4_PLASMA_LIBS )
    target_link_libraries( kdeinit_kor ${KDE4_PLASMA_LIBS} )
else( KDE4_PLASMA_LIBS )
    target_link_libraries( kdeinit_kor plasma )
endif( KDE4_PLASMA_LIBS )

# for plasma systray workaround
target_link_libraries( kdeinit_kor ${X11_Xrender_LIB} )

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
