add_definitions(-DTRANSLATION_DOMAIN=\"processcore\")

########### next target ###############

set(ksysguard_LIB_SRCS
   extended_process_list.cpp
   formatter.cpp
   processes.cpp
   process.cpp
   process_attribute.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
   processes_atop_p.cpp
   process_data_provider.cpp
   unit.cpp
)
ecm_qt_declare_logging_category(ksysguard_LIB_SRCS HEADER processcore_debug.h IDENTIFIER LIBKSYSGUARD_PROCESSCORE CATEGORY_NAME org.kde.libksysguard.processcore)

add_library(processcore ${ksysguard_LIB_SRCS})
add_library(KF5::ProcessCore ALIAS processcore)

target_link_libraries(processcore
   PUBLIC
    Qt5::Core
   PRIVATE
    KF5::I18n
    KF5::CoreAddons
    ${ZLIB_LIBRARIES}
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif()

target_include_directories(processcore
    PUBLIC
        "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard>"
)
set_target_properties(processcore PROPERTIES VERSION ${KSYSGUARD_VERSION_STRING} SOVERSION ${KSYSGUARD_SOVERSION} EXPORT_NAME ProcessCore)
install(TARGETS processcore EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES
    processes.h
    process.h
    process_attribute.h
    process_data_provider.h
    formatter.h
    unit.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/processcore
    COMPONENT Devel
)

#------ KAuth stuff

# Auth example helper
set(ksysguardprocesslist_helper_srcs
    helper.cpp
    process.cpp
    processes_local_p.cpp
    processes_base_p.cpp)

add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
target_link_libraries(ksysguardprocesslist_helper Qt5::Core KF5::AuthCore KF5::I18n)
install(TARGETS ksysguardprocesslist_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
kauth_install_actions(org.kde.ksysguard.processlisthelper actions.actions)

set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

