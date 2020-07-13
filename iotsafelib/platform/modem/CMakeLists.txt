add_library (iotsafeplatform "src/ATInterface.cpp" "src/GenericModem.cpp" "src/LSerial.cpp" "src/Serial.cpp")

target_include_directories (iotsafeplatform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(iotsafeplatform PRIVATE iotsafecommon)