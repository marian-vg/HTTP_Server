cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Agregamos el path manualmente antes de buscar
set(ZLIB_ROOT "C:/Users/Administrador/Documents/vcpkg/installed/x64-windows")

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(server ${SOURCE_FILES})

if (WIN32)
  target_link_libraries(server PRIVATE ws2_32)
endif()

target_include_directories(server PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)
