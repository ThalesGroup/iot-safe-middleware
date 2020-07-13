add_executable(iotsafetests "src/rot_tests_unit_runner.cpp" "src/rot_tests_unit_applet_tests.cpp" "src/rot_tests_helper.c")
target_include_directories (iotsafetests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(iotsafetests PRIVATE iotsafecommon iotsafeplatform CppUTest CppUTestExt)
add_test(NAME run_iotsafetests COMMAND iotsafetests)

