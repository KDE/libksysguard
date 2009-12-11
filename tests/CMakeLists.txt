set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../
${CMAKE_CURRENT_SOURCE_DIR}/../processcore/
${CMAKE_CURRENT_SOURCE_DIR}/../processui/ )


kde4_add_unit_test(processtest processtest.cpp)

target_link_libraries(processtest processui ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})



