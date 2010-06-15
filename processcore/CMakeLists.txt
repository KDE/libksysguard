
########### next target ###############

set(ksysguard_LIB_SRCS
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
   processes_atop_p.cpp
)



kde4_add_library(processcore SHARED ${ksysguard_LIB_SRCS})

target_link_libraries(processcore ${KDE4_KDECORE_LIBS} ${ZLIB_LIBRARIES} )
target_link_libraries(processcore LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )

set_target_properties(processcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS processcore EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)

#------ KAuth stuff

# Auth example helper
set(ksysguardprocesslist_helper_srcs
    helper.cpp
    process.cpp
    processes_local_p.cpp
    processes_base_p.cpp)

kde4_add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
target_link_libraries(ksysguardprocesslist_helper ${KDE4_KDECORE_LIBS})
install(TARGETS ksysguardprocesslist_helper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
kde4_install_auth_actions(org.kde.ksysguard.processlisthelper actions.actions)

set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

