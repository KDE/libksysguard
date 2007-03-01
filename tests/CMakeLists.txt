if(KDE4_BUILD_TESTS)

  include_directories( ${KDE4_INCLUDES} )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  ########### konqviewmgrtest ###############

  kde4_automoc(processtest.cpp)

  kde4_add_executable(processtest RUN_UNINSTALLED processtest.cpp)

  target_link_libraries(processtest solidstats ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})


  ############################################

endif(KDE4_BUILD_TESTS)

