include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_qml_module(SensorsPlugin URI "org.kde.ksysguard.sensors" VERSION 1.0)

target_sources(SensorsPlugin PRIVATE SensorsPlugin.cpp SensorsPlugin.h)

target_link_libraries(SensorsPlugin PRIVATE Qt::Qml KSysGuard::Sensors)

ecm_finalize_qml_module(SensorsPlugin)
