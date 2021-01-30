find_package(Qt5 REQUIRED CONFIG COMPONENTS Test)

include_directories(${libksysguard_SOURCE_DIR})

# Process unit test
ecm_qt_declare_logging_category(processtest_debug_SRCS HEADER processcore_debug.h IDENTIFIER LIBKSYSGUARD_PROCESSCORE CATEGORY_NAME org.kde.libksysguard.processcore)
ecm_add_test(processtest.cpp ${processtest_debug_SRCS} TEST_NAME processtest
	LINK_LIBRARIES KSysGuard::ProcessUi Qt::Test)

if (KF5Plasma_FOUND)
    set(SIGNALPLOTTER_DEBUG_SRCS)
    ecm_qt_declare_logging_category(SIGNALPLOTTER_DEBUG_SRCS HEADER ksignalplotter_debug.h IDENTIFIER LIBKSYSGUARD_KSIGNALPLOTTER CATEGORY_NAME org.kde.libksysguard.ksignalplotter)
    ecm_add_test(signalplotterbenchmark.cpp  ../signalplotter/ksignalplotter.cpp ${SIGNALPLOTTER_DEBUG_SRCS}
        TEST_NAME signalplotterbenchmark
        LINK_LIBRARIES KSysGuard::SignalPlotter Qt::Test Qt::Widgets KF5::IconThemes
    )

    ecm_add_test(graphicssignalplotterbenchmark.cpp ../signalplotter/kgraphicssignalplotter.cpp ${SIGNALPLOTTER_DEBUG_SRCS}
        TEST_NAME graphicssignalplotterbenchmark
        LINK_LIBRARIES KSysGuard::SignalPlotter Qt::Test Qt::Widgets KF5::IconThemes KF5::Plasma
    )

    ecm_add_test(signalplottertest.cpp ../signalplotter/ksignalplotter.cpp ${SIGNALPLOTTER_DEBUG_SRCS}
        TEST_NAME signalplottertest
        LINK_LIBRARIES KSysGuard::SignalPlotter Qt::Test Qt::Widgets KF5::IconThemes
    )
endif()

ecm_add_test(chronotest.cpp
    TEST_NAME chronotest
    LINK_LIBRARIES Qt::Test KF5::I18n
)

ecm_add_test(formattertest.cpp LINK_LIBRARIES Qt::Test KSysGuard::Formatter)
ecm_add_test(sensortreemodeltest.cpp LINK_LIBRARIES Qt::Test Qt::DBus KSysGuard::Sensors)
ecm_add_test(sensordatamodeltest.cpp LINK_LIBRARIES Qt::Test Qt::DBus KSysGuard::Sensors KSysGuard::Formatter)
ecm_add_test(processdatamodeltest.cpp LINK_LIBRARIES Qt::Test KSysGuard::ProcessCore)
ecm_add_test(cgrouptest.cpp LINK_LIBRARIES Qt::Test KF5::Service KSysGuard::ProcessCore)

include_directories(../faces)
ecm_add_test(sensorfacecontrollertest.cpp LINK_LIBRARIES
    Qt::Test
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::I18n
    KF5::Package
    KF5::Declarative
    KF5::Solid
    KSysGuard::SensorFaces
)

# set( ksysguarddtest_SRCS ksysguarddtest.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorManager.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorSocketAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorShellAgent.cpp)
#
# ecm_add_test(${ksysguarddtest_SRCS}
#     TEST_NAME "ksysguard-ksysguarddtest"
#     LINK_LIBRARIES
#     Qt::Test
#     Qt::Network
#     Qt::Widgets
#     KF5::ConfigCore
#     KF5::CoreAddons
#     KF5::I18n
# )
