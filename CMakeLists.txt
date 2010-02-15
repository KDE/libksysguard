
########### next target ###############

kde4_no_enable_final( signalplotter )

add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
add_subdirectory( signalplotter )
add_subdirectory( tests )

find_library(XRES_LIBRARY XRes)
#Do I need Xext as well?  I don't know
#find_library(XEXT_LIBRARY Xext)
if(XRES_LIBRARY)
    set(HAVE_XRES 1)
endif(XRES_LIBRARY)

configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )
install(DIRECTORY scripts/ DESTINATION ${DATA_INSTALL_DIR}/ksysguard/scripts PATTERN .svn EXCLUDE)


