include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(SensorsPlugin SHARED SensorsPlugin.cpp)

target_link_libraries(SensorsPlugin Qt5::Qml KSysGuard::Sensors KF5::ProcessCore KF5::Package KF5::ConfigCore KF5::ConfigGui)

install(TARGETS SensorsPlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/sensors)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/sensors)
