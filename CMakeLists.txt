cmake_minimum_required(VERSION 3.0)

project(libksysguard)
set(PROJECT_VERSION "5.14.80")
set(PROJECT_VERSION_MAJOR 5)

# check with non-Plasma consumers (e.g. KDevelop) before bumping these versions
set(QT_MIN_VERSION "5.5.0")
set(KF5_MIN_VERSION "5.30.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)

include(CMakePackageConfigHelpers)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS DBus Network Widgets)
find_package(Qt5WebEngineWidgets ${QT_MIN_VERSION} CONFIG)
set_package_properties(Qt5WebEngineWidgets PROPERTIES
                       URL "git://code.qt.org/qt/qtwebenginewidgets.git"
                       DESCRIPTION "Qt WebEngine module (web browsing engine)"
                       TYPE OPTIONAL
                       PURPOSE "Used by the HTML-based GUI ksysguard library"
                      )

find_package(KF5 REQUIRED COMPONENTS CoreAddons Config I18n WindowSystem Completion Auth WidgetsAddons IconThemes ConfigWidgets Service GlobalAccel KIO)
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

check_library_exists(c clock_gettime "time.h" HAVE_CLOCK_GETTIME_C)

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
set(HAVE_QTWEBENGINEWIDGETS ${Qt5WebEngineWidgets_FOUND})
configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

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

install(DIRECTORY scripts/ DESTINATION ${KDE_INSTALL_DATADIR}/ksysguard/scripts)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_LIBDIR}/cmake/KF5SysGuard)
configure_package_config_file(KF5SysGuardConfig.cmake.in
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

install(FILES libksysguard.categories DESTINATION ${KDE_INSTALL_CONFDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
