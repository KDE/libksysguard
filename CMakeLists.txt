project(libksysguard)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets WebKitWidgets Script Test)
find_package(KF5 REQUIRED CONFIG COMPONENTS I18n KDE4Support Plasma Config)
find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMAddTests)
include(ECMInstallIcons)
include(CheckIncludeFiles)

set(LIBKSYSGUARD_VERSION_MAJOR 4)
set(LIBKSYSGUARD_VERSION_MINOR 90)
set(LIBKSYSGUARD_VERSION_PATCH 1)
set(LIBKSYSGUARD_VERSION_STRING "${LIBKSYSGUARD_VERSION_MAJOR}.${LIBKSYSGUARD_VERSION_MINOR}.${LIBKSYSGUARD_VERSION_PATCH}" )
write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfigVersion.cmake VERSION
                                     "${LIBKSYSGUARD_VERSION_STRING}" COMPATIBILITY AnyNewerVersion)

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

set(HAVE_XRES ${X11_XRes_FOUND})
configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )

add_subdirectory( lsofui )
add_subdirectory( processcore )
if(Qt5WebKitWidgets_FOUND)
    add_subdirectory( processui )
endif()
add_subdirectory( signalplotter )
add_subdirectory( ksgrd )
add_subdirectory( tests )

install(DIRECTORY scripts/ DESTINATION ${DATA_INSTALL_DIR}/ksysguard/scripts)

set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KF5SysGuard)

configure_file(KF5SysGuardConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfig.cmake" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KF5SysGuardConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(EXPORT libksysguardLibraryTargets
        NAMESPACE KF5::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KF5SysGuardLibraryTargets.cmake )

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
