
########### next target ###############

set(lsofui_LIB_SRCS
   lsof.cpp
)

kde4_add_ui_files( lsofui_LIB_SRCS
   LsofSearchWidget.ui 
)

add_library(lsofui SHARED ${lsofui_LIB_SRCS})

target_link_libraries(lsofui KF5::KI18n Qt5::Widgets)
set_target_properties(lsofui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS lsofui EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES lsof.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)

# set(ksysguardlsofwidgets_PART_SRCS
#    ${CMAKE_CURRENT_BINARY_DIR}/ksysguardlsofwidgets.cpp
# )
#
# kde4_add_widget_files(ksysguardlsofwidgets_PART_SRCS ksysguardlsof.widgets)
#
# add_library(ksysguardlsofwidgets MODULE ${ksysguardlsofwidgets_PART_SRCS})
#
# target_link_libraries(ksysguardlsofwidgets lsofui)
#
# install(TARGETS ksysguardlsofwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

