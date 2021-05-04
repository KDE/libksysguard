
########### next target ###############

set(lsofui_LIB_SRCS
   lsof.cpp
)

ki18n_wrap_ui( lsofui_LIB_SRCS
   LsofSearchWidget.ui 
)

add_library(lsofui ${lsofui_LIB_SRCS})
add_library(KSysGuard::LsofUi ALIAS lsofui)

target_link_libraries(lsofui
    PUBLIC
        Qt::Widgets
    PRIVATE
        KF5::I18n
)
set_target_properties(lsofui PROPERTIES
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${KSYSGUARD_SOVERSION}
    EXPORT_NAME LsofUi
)
install(TARGETS lsofui EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES lsof.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard COMPONENT Devel)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()
