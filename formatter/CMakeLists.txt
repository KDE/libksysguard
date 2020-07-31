
set(Formatter_SOVERSION 1)

set(formatter_SRCS
    Formatter.cpp
    Unit.cpp
)

set(formatter_HDRS
    Formatter.h
    Unit.h
)

add_library(Formatter ${formatter_SRCS})
add_library(KSysGuard::Formatter ALIAS Formatter)

target_link_libraries(Formatter
    PUBLIC
    Qt5::Core
    PRIVATE
    KF5::I18n
    KF5::CoreAddons
)

generate_export_header(Formatter)

target_include_directories(Formatter
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

set_target_properties(Formatter PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardFormatter
    VERSION ${KSYSGUARD_VERSION_STRING}
    SOVERSION ${Formatter_SOVERSION}
)

add_subdirectory(declarative)

install(TARGETS Formatter EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${formatter_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/formatter_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/formatter
)
