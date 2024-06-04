include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(FacesPlugin SHARED FacesPlugin.cpp FacesPlugin.h)

target_link_libraries(FacesPlugin Qt::Qml KSysGuard::Sensors KSysGuard::SensorFaces  KF6::Package KF6::ConfigCore KF6::ConfigGui  KF6::ConfigQml)

install(TARGETS FacesPlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/faces)
install(FILES
    qmldir
    ExtendedLegend.qml
    SensorFace.qml
    SensorRangeSpinBox.qml
    Choices.qml
    CompactSensorFace.qml
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/faces
)
