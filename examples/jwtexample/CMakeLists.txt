
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(jwtexample "src/jwtexample.cpp" "src/sim-access-util.cpp" "src/openssl-util.cpp")
target_include_directories (jwtexample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(jwtexample PRIVATE iotsafecommon iotsafeplatform OpenSSL::Crypto)
 
