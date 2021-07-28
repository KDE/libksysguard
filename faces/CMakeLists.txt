set(KSYSGUARD_SENSORS_SOVERSION 1)

add_subdirectory(import)
add_subdirectory(facepackages)
add_subdirectory(packagestructure)

add_definitions(-DTRANSLATION_DOMAIN=\"KSysGuardSensorFaces\")

set(faces_LIB_SRCS
    SensorFaceController.cpp
    SensorFace.cpp
    FaceLoader.cpp
    resources.qrc
)

set(faces_LIB_HEADERS
    SensorFace_p.h
    SensorFaceController.h
    FaceLoader.h
)

ecm_qt_declare_logging_category(faces_LIB_SRCS
    HEADER faces_logging.h
    IDENTIFIER LIBKSYSGUARD_SENSORS
    CATEGORY_NAME org.kde.libksysguard.faces
    DESCRIPTION "libksysguard (faces)" EXPORT LIBKSYSGUARD)

set_source_files_properties(org.kde.KSysGuardDaemon.xml PROPERTIES INCLUDE SensorInfo_p.h)

add_library(SensorFaces ${faces_LIB_SRCS})
add_library(KSysGuard::SensorFaces ALIAS SensorFaces)

target_include_directories(SensorFaces
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

generate_export_header(SensorFaces)

target_link_libraries(SensorFaces
    PUBLIC
        Qt::Qml
        Qt::Quick
        KF5::ConfigCore
        KF5::ConfigGui
        KSysGuard::Formatter
        KSysGuard::Sensors
    PRIVATE
        Qt::Core
        Qt::DBus
        KF5::I18n
        KF5::Package
        KF5::Declarative
        KF5::Solid
)

set_target_properties(SensorFaces PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSensorFaces
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${KSYSGUARD_SENSORS_SOVERSION}
)

install(TARGETS SensorFaces EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${faces_LIB_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/sensorfaces_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/faces
    COMPONENT Devel
)

install(FILES systemmonitor-faces.knsrc systemmonitor-presets.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
