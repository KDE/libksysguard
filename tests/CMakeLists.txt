set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../
${CMAKE_CURRENT_SOURCE_DIR}/../processcore/
${CMAKE_CURRENT_SOURCE_DIR}/../processui/ )

if(Qt5WebKitWidgets_FOUND)
# Process unit test
kde4_add_unit_test(processtest TESTNAME ksysguard-processtest processtest.cpp)
target_link_libraries(processtest processui  Qt5::Test)
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




