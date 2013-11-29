
set(ksignalplotter_WIDGETS "ksignalplotter.widgets")

set(ksignalplotter_LIB_SRCS
    ksignalplotter.cpp
    kgraphicssignalplotter.cpp
)
# kde4_moc_headers(ksignalplotter ksignalplotter.h kgraphicssignalplotter.h)
add_library(ksignalplotter SHARED ${ksignalplotter_LIB_SRCS})

target_link_libraries(ksignalplotter KF5::KDE4Support KF5::Plasma)

set_target_properties(ksignalplotter
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS ksignalplotter EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

# kde4_add_widget_files(ksignalplotterwidgets_PART_SRCS ${ksignalplotter_WIDGETS})
#
# add_library(ksignalplotterwidgets MODULE ${ksignalplotterwidgets_PART_SRCS})
#
# target_link_libraries(ksignalplotterwidgets ${KDE4_KDEUI_LIBS} ksignalplotter)
#
# install(TARGETS ksignalplotterwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)

install(FILES ksignalplotter.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)
