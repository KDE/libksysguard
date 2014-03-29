
########### next target ###############

set(lsofui_LIB_SRCS
   lsof.cpp
)

qt5_wrap_ui( lsofui_LIB_SRCS
   LsofSearchWidget.ui 
)

add_library(lsofui ${lsofui_LIB_SRCS})
add_library(KF5::LsofUi ALIAS lsofui)

target_link_libraries(lsofui
    PUBLIC
        Qt5::Widgets
        KF5::KDE4Support
    PRIVATE
        KF5::I18n
)
set_target_properties(lsofui PROPERTIES VERSION ${LIBKSYSGUARD_VERSION_STRING} SOVERSION ${LIBKSYSGUARD_VERSION_MINOR} EXPORT_NAME LsofUi)
install(TARGETS lsofui EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

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

