set(KSYSGUARD_SENSORS_SOVERSION 1)

add_subdirectory(declarative)

add_definitions(-DTRANSLATION_DOMAIN=\"ksysguard_sensors\")

set(sensors_LIB_SRCS
    Sensor.cpp
    SensorDataModel.cpp
    SensorTreeModel.cpp
    SensorQuery.cpp
    SensorDaemonInterface.cpp
    SensorFaceController.cpp
    SensorFace.cpp
)

set(sensors_LIB_HEADERS
    Sensor.h
    SensorDataModel.h
    SensorTreeModel.h
    SensorQuery.h
    SensorInfo_p.h
    SensorFaceController.h
)

ecm_qt_declare_logging_category(sensors_LIB_SRCS
    HEADER sensors_logging.h
    IDENTIFIER LIBKSYSGUARD_SENSORS
    CATEGORY_NAME org.kde.libksysguard.sensors
)

set_source_files_properties(org.kde.KSysGuardDaemon.xml PROPERTIES INCLUDE SensorInfo_p.h)
qt5_add_dbus_interface(sensors_LIB_SRCS org.kde.KSysGuardDaemon.xml ksysguarddaemon)
qt5_add_resources(RESOURCES resources.qrc)

add_library(Sensors ${sensors_LIB_SRCS} ${RESOURCES})
add_library(KSysGuard::Sensors ALIAS Sensors)

target_include_directories(Sensors
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

generate_export_header(Sensors)

target_link_libraries(Sensors
    PUBLIC
        Qt5::Qml
        Qt5::Quick
        KSysGuard::Formatter
    PRIVATE
        Qt5::Core
        Qt5::DBus
        KF5::I18n
        KF5::Package
        KF5::ConfigCore
        KF5::ConfigGui
        KF5::Declarative
)

set_target_properties(Sensors PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSensors
    VERSION ${KSYSGUARD_VERSION_STRING}
    SOVERSION ${KSYSGUARD_SENSORS_SOVERSION}
)

install(TARGETS Sensors EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${sensors_LIB_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/sensors_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/sensors
    COMPONENT Devel
)
