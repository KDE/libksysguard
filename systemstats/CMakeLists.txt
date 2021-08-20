set(SystemStats_SOVERSION 1)

set(systemstats_SRCS
    AggregateSensor.cpp
    SensorObject.cpp
    SensorContainer.cpp
    SensorPlugin.cpp
    SensorProperty.cpp
    SysFsSensor.cpp
    DBusInterface.cpp
)

set(systemstats_HDRS
    AggregateSensor.h
    SensorObject.h
    SensorContainer.h
    SensorPlugin.h
    SensorProperty.h
    SensorsFeatureSensor.h
    SysFsSensor.h
    SysctlSensor.h
    SensorInfo.h
    DBusInterface.h
)

set_source_files_properties(org.kde.ksystemstats.xml PROPERTIES INCLUDE SensorInfo.h)
qt_add_dbus_interface(dbusinterface_SRCS org.kde.ksystemstats.xml org.kde.ksystemstats)

# Symbol visibility trickery: We cannot generate the DBus interface class with
# the right export macro, so instead create it as a static library with the right
# visibility preset set, then link to that in the shared library.
add_library(dbusinterface STATIC ${dbusinterface_SRCS})
set_target_properties(dbusinterface PROPERTIES CXX_VISIBILITY_PRESET default)
target_link_libraries(dbusinterface Qt::Core Qt::DBus KSysGuard::Formatter)

add_library(SystemStats ${systemstats_SRCS})
add_library(KSysGuard::SystemStats ALIAS SystemStats)

target_include_directories(SystemStats
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

target_link_libraries(SystemStats PUBLIC Qt::Core Qt::DBus KSysGuard::Formatter PRIVATE dbusinterface)

if (SENSORS_FOUND)
    target_sources(SystemStats PRIVATE SensorsFeatureSensor.cpp)
    target_link_libraries(SystemStats PUBLIC ${SENSORS_LIBRARIES} KF5::I18n)
    target_include_directories(SystemStats PUBLIC ${SENSORS_INCLUDE_DIR})
endif()

generate_export_header(SystemStats)

set_target_properties(SystemStats PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSystemStats
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${SystemStats_SOVERSION}
)

install(TARGETS SystemStats EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${systemstats_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/systemstats_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ksystemstats.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/systemstats
)

install(FILES org.kde.ksystemstats.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
