
set(ksignalplotter_WIDGETS "ksignalplotter.widgets")

set(ksignalplotter_LIB_SRCS
    ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp
    ksignalplotter.cpp
    kgraphicssignalplotter.cpp
)
# kde4_moc_headers(ksignalplotter ksignalplotter.h kgraphicssignalplotter.h)
add_library(ksignalplotter ${ksignalplotter_LIB_SRCS})
add_library(KF5::SignalPlotter ALIAS ksignalplotter)

target_link_libraries(ksignalplotter
    PUBLIC
        Qt5::Widgets
        KF5::I18n # klocalizedstring.h
        KF5::IconThemes
    PRIVATE
        KF5::ProcessCore
        KF5::Plasma
)

set_target_properties(ksignalplotter
   PROPERTIES VERSION ${KSYSGUARD_VERSION_STRING}
   SOVERSION ${KSYSGUARD_SOVERSION}
   EXPORT_NAME SignalPlotter
)

install(TARGETS ksignalplotter EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

# kde4_add_widget_files(ksignalplotterwidgets_PART_SRCS ${ksignalplotter_WIDGETS})
#
# add_library(ksignalplotterwidgets MODULE ${ksignalplotterwidgets_PART_SRCS})
#
# target_link_libraries(ksignalplotterwidgets ${KDE4_KDEUI_LIBS} ksignalplotter)
#
# install(TARGETS ksignalplotterwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)

install(FILES ksignalplotter.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)
