


########### next target ###############

set(ksgrd_LIB_SRCS
   SensorAgent.cpp
   SensorManager.cpp
   SensorShellAgent.cpp
   SensorSocketAgent.cpp
)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

add_library(ksgrd SHARED ${ksgrd_LIB_SRCS})

target_link_libraries(ksgrd
    Qt5::Core
    Qt5::Network
    KF5::I18n
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::KDE4Support
)

set_target_properties(ksgrd PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ksgrd EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install(FILES SensorAgent.h SensorClient.h SensorManager.h SensorShellAgent.h SensorSocketAgent.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksgrd COMPONENT Devel)



