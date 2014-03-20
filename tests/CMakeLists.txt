include_directories(${libksysguard_SOURCE_DIR})
if(Qt5WebKitWidgets_FOUND)
    # Process unit test
    kde4_add_unit_test(processtest TESTNAME ksysguard-processtest processtest.cpp)
    target_link_libraries(processtest KF5::ProcessUi Qt5::Test)
endif()

# KSignalPlotter benchmark
set( signalplotterbenchmark_SRCS signalplotterbenchmark.cpp ../signalplotter/ksignalplotter.cpp)
kde4_add_unit_test( signalplotterbenchmark TESTNAME ksysguard-signalplottertest ${signalplotterbenchmark_SRCS} )
target_link_libraries( signalplotterbenchmark Qt5::Test Qt5::Widgets KF5::KDE4Support )


# KGraphicsSignalPlotter benchmark
set( graphicssignalplotterbenchmark_SRCS graphicssignalplotterbenchmark.cpp ../signalplotter/kgraphicssignalplotter.cpp)
kde4_add_unit_test( graphicssignalplotterbenchmark TESTNAME ksysguard-signalplottertest ${graphicssignalplotterbenchmark_SRCS} )
target_link_libraries( graphicssignalplotterbenchmark Qt5::Test Qt5::Widgets KF5::KDE4Support KF5::Plasma )


# KSignalPlotter unit test
set( signalplottertest_SRCS signalplottertest.cpp ../signalplotter/ksignalplotter.cpp)
kde4_add_unit_test( signalplottertest TESTNAME ksysguard-signalplottertest ${signalplottertest_SRCS} )
target_link_libraries( signalplottertest Qt5::Test Qt5::Widgets KF5::I18n KF5::KDE4Support )


set( ksysguarddtest_SRCS ksysguarddtest.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorManager.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorSocketAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorShellAgent.cpp)

ecm_add_test(${ksysguarddtest_SRCS}
    TEST_NAME "ksysguard-ksysguarddtest"
    LINK_LIBRARIES
    Qt5::Test
    Qt5::Network
    Qt5::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
)


