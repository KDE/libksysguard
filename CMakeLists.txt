project(libksysguard)
set(PROJECT_VERSION "5.9.90")
set(PROJECT_VERSION_MAJOR 5)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 REQUIRED CONFIG COMPONENTS DBus Network Widgets Script)
find_package(Qt5WebKitWidgets CONFIG)
set_package_properties(Qt5WebKitWidgets PROPERTIES
                       URL "git://gitorious.org/qt/qtwebkit.git"
                       DESCRIPTION "Qt Webkit module (web browsing engine)"
                       TYPE OPTIONAL
                       PURPOSE "Used by the HTML-based GUI ksysguard library"
                      )

find_package(KF5 REQUIRED COMPONENTS CoreAddons Config I18n WindowSystem Completion Auth WidgetsAddons IconThemes ConfigWidgets Service)
find_package(KF5 OPTIONAL_COMPONENTS Plasma)
set_package_properties(KF5Plasma PROPERTIES
                       URL "https://cgit.kde.org/plasma-framework.git/"
                       DESCRIPTION "The library of the plasma project"
                       TYPE OPTIONAL
                       PURPOSE "Used by signalplotter to use Plasma themes"
                      )

find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMPackageConfigHelpers)
include(CheckIncludeFiles)
include(ECMSetupVersion)

ecm_setup_version(${PROJECT_VERSION}
                  VARIABLE_PREFIX KSYSGUARD
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksysguard_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfigVersion.cmake"
                  SOVERSION 7
)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace"
                    )
if(X11_FOUND)
    find_package(Qt5X11Extras REQUIRED)
    find_library(X11_XRes_LIB XRes ${X11_LIB_SEARCH_PATH})
    find_path(X11_XRes_INCLUDE_PATH X11/extensions/XRes.h ${X11_INC_SEARCH_PATH})

    if(X11_XRes_LIB AND X11_XRes_INCLUDE_PATH)
        set(X11_XRes_FOUND TRUE)
    endif()
endif()

set(HAVE_X11 ${X11_FOUND})
set(HAVE_XRES ${X11_XRes_FOUND})
set(HAVE_QTWEBKITWIDGETS ${Qt5WebKitWidgets_FOUND})
configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )

add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
if (KF5Plasma_FOUND)
    add_subdirectory( signalplotter )
endif()
add_subdirectory( ksgrd )

if(BUILD_TESTING)
add_subdirectory( tests )
endif()

install(DIRECTORY scripts/ DESTINATION ${DATA_INSTALL_DIR}/ksysguard/scripts)

set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KF5SysGuard)
ecm_configure_package_config_file(KF5SysGuardConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfig.cmake
		${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(EXPORT libksysguardLibraryTargets
        NAMESPACE KF5::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KF5SysGuardLibraryTargets.cmake )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
