cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

cmake_policy(VERSION 3.13)

project ("ll-bridge")

#set up cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(thirdparty)
add_subdirectory(libs)
add_subdirectory(apps/server)
add_subdirectory(tests)
