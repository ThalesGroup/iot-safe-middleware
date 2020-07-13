add_library (iotsafecommon "src/Applet.cpp" "src/ROT.cpp" "src/SEInterface.cpp")

target_include_directories (iotsafecommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
