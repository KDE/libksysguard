add_library(sensorface_packagestructure MODULE sensorfacepackage.cpp)

target_link_libraries(sensorface_packagestructure PRIVATE KF5::Package KF5::I18n)

install(TARGETS sensorface_packagestructure DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)
