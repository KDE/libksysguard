
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../  ../processcore/)

check_include_files(sys/ptrace.h HAVE_SYS_PTRACE_H)
check_include_files(byteswap.h HAVE_BYTESWAP_H)

if (HAVE_SYS_PTRACE_H)
   set(processui_ptrace_SRCS KMonitorProcessIO.cpp DisplayProcessDlg.cpp)
else (HAVE_SYS_PTRACE_H)
   # Even if we don't have ptrace, we need KMonitorProcessIO.cpp
   # because the widget is referenced from the .widgets file and
   # from the .ui files.
   set(processui_ptrace_SRCS KMonitorProcessIO.cpp)
endif (HAVE_SYS_PTRACE_H)

set(processui_LIB_SRCS
   ksysguardprocesslist.cpp
   ProcessFilter.cc
   ProcessModel.cc
   ReniceDlg.cc
   KTextEditVT.cpp
   ${processui_ptrace_SRCS}
)

kde4_add_ui_files( processui_LIB_SRCS
   ReniceDlgUi.ui
   ProcessWidgetUI.ui
   DisplayProcessUi.ui
)


kde4_add_library(processui SHARED ${processui_LIB_SRCS})

target_link_libraries(processui ${KDE4_KIO_LIBS} processcore)

set_target_properties(processui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )


install(TARGETS processui ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

set(ksysguardwidgets_PART_SRCS
   ${CMAKE_CURRENT_BINARY_DIR}/ksysguardwidgets.cpp
)

KDE4_ADD_WIDGET_FILES(${ksysguardwidgets_PART_SRCS} ksysguard.widgets)

kde4_add_plugin(ksysguardwidgets ${ksysguardwidgets_PART_SRCS})

target_link_libraries(ksysguardwidgets ${KDE4_KIO_PARTS} processui)

install(TARGETS ksysguardwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

install( FILES ProcessModel.h DisplayProcessDlg.h ProcessFilter.h KTextEditVT.h ksysguardprocesslist.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard)
