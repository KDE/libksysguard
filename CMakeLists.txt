cmake_minimum_required(VERSION 3.16)

project(libksysguard)

set(PROJECT_VERSION "5.27.80")
set(PROJECT_VERSION_MAJOR 6)

# check with non-Plasma consumers (e.g. KDevelop) before bumping these versions to make sure the KDE CI does not break
set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "5.240.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)

include(CMakePackageConfigHelpers)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FeatureSummary)
include(GenerateExportHeader)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS DBus Network Widgets Qml Quick Core5Compat)
find_package(Qt6WebEngineWidgets ${QT_MIN_VERSION} CONFIG)
set_package_properties(Qt6WebEngineWidgets PROPERTIES
                       URL "git://code.qt.org/qt/qtwebenginewidgets.git"
                       DESCRIPTION "Qt WebEngine module (web browsing engine)"
                       TYPE OPTIONAL
                       PURPOSE "Used by the HTML-based GUI ksysguard library"
                      )
find_package(Qt6WebChannel ${QT_MIN_VERSION} CONFIG)
set_package_properties(Qt6WebChannel PROPERTIES
                       URL "git://code.qt.org/qt/qtwebchannel.git"
                       DESCRIPTION "Qt WebChannel module"
                       TYPE OPTIONAL
                       PURPOSE "Used by the HTML-based GUI ksysguard library"
                      )


find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config I18n JobWidgets WindowSystem Auth WidgetsAddons IconThemes ConfigWidgets Service GlobalAccel KIO Package NewStuff Solid)

find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "https://zlib.net/"
                       TYPE REQUIRED
                      )

check_library_exists(c clock_gettime "time.h" HAVE_CLOCK_GETTIME_C)

ecm_setup_version(${PROJECT_VERSION}
                  VARIABLE_PREFIX KSYSGUARD
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksysguard_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfigVersion.cmake"
                  SOVERSION 9
)

option(WITH_X11 "Build with X11 support." ON)
if(WITH_X11)
    find_package(X11)
    set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                            URL "https://www.x.org"
                            TYPE OPTIONAL
                            PURPOSE "Required for building Process UI with X11 support"
                        )
    if(X11_FOUND)
        find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Gui) # qtx11extras_p.h
        set(HAVE_X11 1)
        find_library(X11_XRes_LIB XRes ${X11_LIB_SEARCH_PATH})
        find_path(X11_XRes_INCLUDE_PATH X11/extensions/XRes.h ${X11_INC_SEARCH_PATH})

        if(X11_XRes_LIB AND X11_XRes_INCLUDE_PATH)
            set(HAVE_XRES 1)
        endif()
    endif()
else()
    set(HAVE_X11 0)
    set(HAVE_XRES 0)
endif()

set(WEBENGINE_SCRIPTING_ENABLED FALSE)
if(Qt6WebEngineWidgets_FOUND AND Qt6WebChannel_FOUND)
    set(WEBENGINE_SCRIPTING_ENABLED TRUE)
endif()
add_feature_info("Scripting plugin support" WEBENGINE_SCRIPTING_ENABLED "Support scripting plugins using WebEngine and WebChannel")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(NL)
    set_package_properties(NL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Used for gathering socket info via the sock_diag netlink subsystem."
        URL "https://github.com/thom311/libnl/"
    )
    find_package(libpcap)
    set_package_properties(
        libpcap PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "libpcap is used for per-application network usage."
    )

    if(libpcap_FOUND )
        set(BUILD_NETWORK_PLUGIN TRUE)

        find_package(Libcap)
        set_package_properties(Libcap PROPERTIES
            TYPE OPTIONAL
            PURPOSE "Needed for setting capabilities of the per-application network plugin."
        )
    endif()

    find_package(Sensors)
    set_package_properties(Sensors PROPERTIES
        TYPE REQUIRED
        PURPOSE "Used for reading hardware sensors")
endif()

configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )

option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

option(ENABLE_KAUTH_HELPER "Build and install ksysguardprocesslist_helper KAuth helper" ON)
add_feature_info(KAUTH_HELPER ${ENABLE_KAUTH_HELPER} "Build and install ksysguardprocesslist_helper KAuth helper")

ecm_set_disabled_deprecation_versions(
    QT 5.15.0
)

add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055B00)
add_definitions(-DKF_DEPRECATED_WARNINGS_SINCE=0x060000)
add_definitions(-DQT_USE_QSTRINGBUILDER)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory( formatter )
add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
add_subdirectory( sensors )
add_subdirectory( faces )
add_subdirectory( signalplotter )
add_subdirectory( ksgrd )
add_subdirectory( systemstats )

if(BUILD_TESTING)
    add_subdirectory( autotests )
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

install(DIRECTORY scripts/ DESTINATION ${KDE_INSTALL_DATADIR}/ksysguard/scripts)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_LIBDIR}/cmake/KSysGuard)
configure_package_config_file(KSysGuardConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(EXPORT libksysguardLibraryTargets
        NAMESPACE KSysGuard::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KSysGuardLibraryTargets.cmake )

ecm_qt_install_logging_categories(
        EXPORT LIBKSYSGUARD
        FILE libksysguard.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
