
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
)

generate_export_header(Formatter)

set_target_properties(Formatter PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardFormatter
    VERSION ${KSYSGUARD_VERSION_STRING}
    SOVERSION ${Formatter_SOVERSION}
    PUBLIC_HEADER "${formatter_HDRS}"
)

add_subdirectory(declarative)

install(TARGETS Formatter EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS Formatter PUBLIC_HEADER DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/formatter)
