
########### next target ###############

kde4_no_enable_final( signalplotter )

add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
add_subdirectory( signalplotter )
add_subdirectory( tests )

check_include_files(sys/ptrace.h HAVE_SYS_PTRACE_H)

find_library(XRES_LIBRARY XRes)
find_library(X11_LIBRARY  X11)
#Do I need Xext as well?  I don't know
#find_library(XEXT_LIBRARY Xext)
if(XRES_LIBRARY AND X11_LIBRARY)
    set(HAVE_XRES 1)
endif(XRES_LIBRARY AND X11_LIBRARY)

configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )
install(DIRECTORY scripts/ DESTINATION ${DATA_INSTALL_DIR}/ksysguard/scripts PATTERN .svn EXCLUDE)


