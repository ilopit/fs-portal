add_executable(ll-bridge-tests 
    test_llbridge.cpp
    utils_tests.cpp
    fileapi_test.cpp
    test_binary_stream.cpp
    file_cache_test.cpp
    file_recombinator_test.cpp
    test_helpers.h
    test_helpers.cpp
    test_secure_session.cpp
    test_messages.cpp
)

target_link_libraries(ll-bridge-tests PUBLIC engine gtest gtest_main botan)
target_include_directories(ll-bridge-tests PUBLIC "${PROJECT_SOURCE_DIR}/libs/engine/private/include"  )