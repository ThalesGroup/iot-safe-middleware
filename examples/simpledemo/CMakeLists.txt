add_executable(simpledemo "src/simpledemo.cpp" "src/util.cpp")
target_include_directories (simpledemo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(simpledemo PRIVATE iotsafecommon iotsafeplatform)
 
