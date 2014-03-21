include_directories(${libksysguard_SOURCE_DIR})
if(Qt5WebKitWidgets_FOUND)
    # Process unit test
    ecm_add_test(processtest.cpp TEST_NAME processtest
                 LINK_LIBRARIES KF5::ProcessUi Qt5::Test)
endif()

ecm_add_test(signalplotterbenchmark.cpp ../signalplotter/ksignalplotter.cpp
    TEST_NAME signalplotterbenchmark
    LINK_LIBRARIES Qt5::Test Qt5::Widgets KF5::KDE4Support)

ecm_add_test(graphicssignalplotterbenchmark.cpp ../signalplotter/kgraphicssignalplotter.cpp
    TEST_NAME graphicssignalplotterbenchmark
    LINK_LIBRARIES Qt5::Test Qt5::Widgets KF5::KDE4Support KF5::Plasma
)

ecm_add_test(signalplottertest.cpp ../signalplotter/ksignalplotter.cpp
    TEST_NAME signalplottertest
    LINK_LIBRARIES Qt5::Test Qt5::Widgets KF5::I18n KF5::KDE4Support
)


# set( ksysguarddtest_SRCS ksysguarddtest.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorManager.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorSocketAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorShellAgent.cpp)
#
# ecm_add_test(${ksysguarddtest_SRCS}
#     TEST_NAME "ksysguard-ksysguarddtest"
#     LINK_LIBRARIES
#     Qt5::Test
#     Qt5::Network
#     Qt5::Widgets
#     KF5::ConfigCore
#     KF5::CoreAddons
#     KF5::I18n
# )


