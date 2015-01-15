remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)
add_definitions(-DTRANSLATION_DOMAIN=\"processcore\")

########### next target ###############

set(ksysguard_LIB_SRCS
   processcore_debug.cpp
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
   processes_atop_p.cpp
)



add_library(processcore ${ksysguard_LIB_SRCS})
add_library(KF5::ProcessCore ALIAS processcore)

target_link_libraries(processcore
   PUBLIC
    Qt5::Core
   PRIVATE
    KF5::I18n
    ${ZLIB_LIBRARIES}
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif()

target_include_directories(processcore PUBLIC "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>" "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/ksysguard>")
set_target_properties(processcore PROPERTIES VERSION ${KSYSGUARD_VERSION_STRING} SOVERSION ${KSYSGUARD_SOVERSION} EXPORT_NAME ProcessCore)
install(TARGETS processcore EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard/processcore COMPONENT Devel)

#------ KAuth stuff

# Auth example helper
set(ksysguardprocesslist_helper_srcs
    helper.cpp
    process.cpp
    processes_local_p.cpp
    processes_base_p.cpp)

add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
target_link_libraries(ksysguardprocesslist_helper Qt5::Core KF5::Auth KF5::I18n)
install(TARGETS ksysguardprocesslist_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
kauth_install_actions(org.kde.ksysguard.processlisthelper actions.actions)

set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

