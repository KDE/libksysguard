set(SystemStats_SOVERSION 1)

set(systemstats_SRCS
    AggregateSensor.cpp
    SensorObject.cpp
    SensorContainer.cpp
    SensorPlugin.cpp
    SensorProperty.cpp
    SysFsSensor.cpp
)

set(systemstats_HDRS
    AggregateSensor.h
    SensorObject.h
    SensorContainer.h
    SensorPlugin.h
    SensorProperty.h
    SysFsSensor.h
    SysctlSensor.h
    SensorInfo.h
)

add_library(SystemStats ${systemstats_SRCS})
add_library(KSysGuard::SystemStats ALIAS SystemStats)

target_link_libraries(SystemStats PUBLIC Qt::Core Qt::DBus KSysGuard::Formatter)

generate_export_header(SystemStats)

set_target_properties(SystemStats PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSystemStats
    VERSION ${KSYSGUARD_VERSION_STRING}
    SOVERSION ${SystemStats_SOVERSION}
)

install(TARGETS SystemStats EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${systemstats_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/systemstats_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/systemstats
)
