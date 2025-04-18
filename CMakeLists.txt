cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

set(ZLIB_LIBRARY "C:/Users/Administrador/Documents/vcpkg/installed/x64-windows/lib/zlib.lib")
set(ZLIB_INCLUDE_DIR "C:/Users/Administrador/Documents/vcpkg/installed/x64-windows/include")

find_package(ZLIB REQUIRED)

add_executable(server ${SOURCE_FILES})

if (WIN32)
  target_link_libraries(server PRIVATE ws2_32)
endif()

target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)
