
set(ksignalplotter_WIDGETS "ksignalplotter.widgets")

set(ksignalplotter_LIB_SRCS
    ksignalplotter.cpp
    kgraphicssignalplotter.cpp
)
KDE4_MOC_HEADERS(ksignalplotter ksignalplotter.h kgraphicssignalplotter.h)
kde4_add_library(ksignalplotter SHARED ${ksignalplotter_LIB_SRCS})

target_link_libraries(ksignalplotter ${KDE4_KDEUI_LIBS})
target_link_libraries(ksignalplotter LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

set_target_properties(ksignalplotter
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS ksignalplotter EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

kde4_add_widget_files(ksignalplotterwidgets_PART_SRCS ${ksignalplotter_WIDGETS})

kde4_add_plugin(ksignalplotterwidgets ${ksignalplotterwidgets_PART_SRCS})

target_link_libraries(ksignalplotterwidgets ${KDE4_KDEUI_LIBS} ksignalplotter)

install(TARGETS ksignalplotterwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)

install(FILES ksignalplotter.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)
