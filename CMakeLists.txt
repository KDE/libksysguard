
########### next target ###############

kde4_no_enable_final( signalplotter )

if(Q_WS_X11)
    find_library(X11_XRes_LIB XRes ${X11_LIB_SEARCH_PATH})
    find_path(X11_XRes_INCLUDE_PATH X11/extensions/XRes.h ${X11_INC_SEARCH_PATH})

    if(X11_XRes_LIB AND X11_XRes_INCLUDE_PATH)
        set(X11_XRes_FOUND TRUE)
    endif(X11_XRes_LIB AND X11_XRes_INCLUDE_PATH)
endif(Q_WS_X11)

macro_bool_to_01(X11_XRes_FOUND HAVE_XRES)
configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )

add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
add_subdirectory( signalplotter )
add_subdirectory( ksgrd )
add_subdirectory( tests )

install(DIRECTORY scripts/ DESTINATION ${DATA_INSTALL_DIR}/ksysguard/scripts)

