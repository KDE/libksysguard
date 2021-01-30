include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(SensorsPlugin SHARED SensorsPlugin.cpp)

target_link_libraries(SensorsPlugin Qt::Qml KSysGuard::Sensors)

install(TARGETS SensorsPlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/sensors)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/ksysguard/sensors)
