include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(FacesPlugin SHARED FacesPlugin.cpp)

target_link_libraries(FacesPlugin Qt::Qml KSysGuard::Sensors KSysGuard::SensorFaces  KF5::Package KF5::ConfigCore KF5::ConfigGui KF5::Declarative)

install(TARGETS FacesPlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/faces)
install(FILES
    qmldir
    ExtendedLegend.qml
    SensorFace.qml
    SensorRangeSpinBox.qml
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/faces
)
