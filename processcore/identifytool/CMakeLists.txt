
add_executable(ksysguard-identify main.cpp)

target_link_libraries(ksysguard-identify PRIVATE KSysGuard::ProcessCore KF6::CoreAddons)

install(TARGETS ksysguard-identify ${KDE_INSTALL_DEFAULT_ARGUMENTS})
