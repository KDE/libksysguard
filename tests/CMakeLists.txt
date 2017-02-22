find_package(Qt5 REQUIRED CONFIG COMPONENTS Test)

include_directories(${libksysguard_SOURCE_DIR})
if(Qt5WebKitWidgets_FOUND)
    # Process unit test
    ecm_add_test(processtest.cpp ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp TEST_NAME processtest
            LINK_LIBRARIES KF5::ProcessUi Qt5::Test)
endif()

if (KF5Plasma_FOUND)
    ecm_add_test(signalplotterbenchmark.cpp ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp ../signalplotter/ksignalplotter.cpp
        TEST_NAME signalplotterbenchmark
        LINK_LIBRARIES KF5::SignalPlotter Qt5::Test Qt5::Widgets KF5::IconThemes
    )

    ecm_add_test(graphicssignalplotterbenchmark.cpp ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp ../signalplotter/kgraphicssignalplotter.cpp
        TEST_NAME graphicssignalplotterbenchmark
        LINK_LIBRARIES KF5::SignalPlotter Qt5::Test Qt5::Widgets KF5::IconThemes KF5::Plasma
    )

    ecm_add_test(signalplottertest.cpp ${libksysguard_SOURCE_DIR}/processcore/processcore_debug.cpp ../signalplotter/ksignalplotter.cpp
        TEST_NAME signalplottertest
        LINK_LIBRARIES KF5::SignalPlotter Qt5::Test Qt5::Widgets KF5::IconThemes
    )
endif()

ecm_add_test(chronotest.cpp
    TEST_NAME chronotest
    LINK_LIBRARIES Qt5::Test KF5::I18n
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
