add_library(ksysguard_plugin_gpu MODULE gpu.cpp)
target_link_libraries(ksysguard_plugin_gpu Qt::Core KSysGuard::ProcessCore KF6::I18n KF6::CoreAddons Libdrm::Libdrm)
install(TARGETS ksysguard_plugin_gpu DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard/process)
