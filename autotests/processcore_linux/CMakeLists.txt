
include_directories(../../processcore/linux_threaded/)

ecm_add_test(procstatustest.cpp LINK_LIBRARIES Qt5::Test KSysGuard::ProcessCore processcore_linux)
