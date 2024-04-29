include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_qml_module(FormatterPlugin URI "org.kde.ksysguard.formatter" VERSION 1.0)

target_sources(FormatterPlugin PRIVATE FormatterPlugin.cpp FormatterPlugin.h FormatterWrapper.cpp FormatterWrapper.h)

target_link_libraries(FormatterPlugin PRIVATE Qt::Qml KSysGuard::Formatter)

ecm_finalize_qml_module(FormatterPlugin)
