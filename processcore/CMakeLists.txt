remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

########### next target ###############

set(ksysguard_LIB_SRCS
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
   processes_atop_p.cpp
)



add_library(processcore ${ksysguard_LIB_SRCS})

target_link_libraries(processcore
   PUBLIC
    KF5::I18n
   PRIVATE
    KF5::KDE4Support
    ${ZLIB_LIBRARIES}
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif()

target_include_directories(processcore PUBLIC $<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>)
set_target_properties(processcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS processcore EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)

#------ KAuth stuff

# Auth example helper
# set(ksysguardprocesslist_helper_srcs
#     helper.cpp
#     process.cpp
#     processes_local_p.cpp
#     processes_base_p.cpp)
#
# add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
# target_link_libraries(ksysguardprocesslist_helper )
# install(TARGETS ksysguardprocesslist_helper DESTINATION ${LIBEXEC_INSTALL_DIR})
#
# kauth_install_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
# kauth_install_actions(org.kde.ksysguard.processlisthelper actions.actions)
#
# set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

