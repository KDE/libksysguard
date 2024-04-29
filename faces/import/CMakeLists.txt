include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_qml_module(FacesPlugin URI "org.kde.ksysguard.faces" VERSION 1.0)

target_sources(FacesPlugin PRIVATE FacesPlugin.cpp FacesPlugin.h)

ecm_target_qml_sources(FacesPlugin SOURCES
    ExtendedLegend.qml
    SensorFace.qml
    SensorRangeSpinBox.qml
    Choices.qml
    CompactSensorFace.qml
)

target_link_libraries(FacesPlugin PRIVATE Qt::Qml KSysGuard::Sensors KSysGuard::SensorFaces  KF6::Package KF6::ConfigCore KF6::ConfigGui  KF6::ConfigQml)

ecm_finalize_qml_module(FacesPlugin)
