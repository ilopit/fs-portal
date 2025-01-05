include(FetchContent)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)

FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.81.0/boost-1.81.0.tar.xz
)

set(BOOST_INCLUDE_LIBRARIES beast url system)
FetchContent_MakeAvailable(Boost)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.zip
)
FetchContent_MakeAvailable(spdlog)

set(BOTAN_MODULES_LIST
    system_rng
    aes_ni
    aead
    gcm
    siv
    pbkdf2
    argon2
)

list(JOIN BOTAN_MODULES_LIST "," ENABLE_MODULES_LIST)

FetchContent_Declare(
    botan_upstream
    URL https://github.com/randombit/botan/archive/refs/tags/3.6.1.tar.gz
)
FetchContent_MakeAvailable(botan_upstream)

# Determine botan compiler ID (--cc parameter of configure.py)
set(BOTAN_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
string(TOLOWER ${BOTAN_COMPILER_ID} BOTAN_COMPILER_ID)
if (BOTAN_COMPILER_ID STREQUAL "gnu")
    set(BOTAN_COMPILER_ID "gcc")
endif()

execute_process(COMMAND python
    ${botan_upstream_SOURCE_DIR}/configure.py
    --quiet
    --cc-bin=${CMAKE_CXX_COMPILER}
    --cc=${BOTAN_COMPILER_ID}
    --disable-shared
    --amalgamation
    --minimized-build
    --enable-modules=${ENABLE_MODULES_LIST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE CONFIG_RESULT)
    if(NOT CONFIG_RESULT EQUAL "0")
        message(FATAL_ERROR "./configure.py failed with ${CONFIG_RESULT}")
    endif()

add_library(botan STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.cpp
)
target_include_directories(botan PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

