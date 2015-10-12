


########### next target ###############

set(ksgrd_LIB_SRCS
   ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp
   SensorAgent.cpp
   SensorManager.cpp
   SensorShellAgent.cpp
   SensorSocketAgent.cpp
)

add_definitions(-DTRANSLATION_DOMAIN=\"ksgrd\")

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

add_library(ksgrd ${ksgrd_LIB_SRCS})
add_library(KF5::SysGuard ALIAS ksgrd)
target_include_directories(ksgrd PUBLIC "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>" "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/ksysguard/>")

target_link_libraries(ksgrd
    PUBLIC
        Qt5::Core
        Qt5::Widgets # QWidget in SensorManager.h
        Qt5::Network # QTcpSocket in SensorSocketAgent.h
        KF5::ConfigCore # KConfigGroup in SensorManager.h
    PRIVATE
        KF5::I18n
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::ProcessCore
)

set_target_properties(ksgrd PROPERTIES VERSION ${KSYSGUARD_VERSION_STRING} SOVERSION ${KSYSGUARD_SOVERSION} EXPORT_NAME SysGuard)
install(TARGETS ksgrd EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES SensorAgent.h SensorClient.h SensorManager.h SensorShellAgent.h SensorSocketAgent.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard/ksgrd COMPONENT Devel)



